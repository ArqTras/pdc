// Copyright (c) 2018-2019 Zano Project
// Copyright (c) 2026 PDC
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.


#include "include_base_utils.h"
using namespace epee;

#include "basic_pow_helpers.h"
#include "currency_format_utils.h"
#include "serialization/binary_utils.h"
#include "serialization/stl_containers.h"
#include "currency_core/currency_config.h"
#include "crypto/crypto.h"
#include "crypto/hash.h"
#include "common/int-util.h"

#include <cstring>
#include <mutex>
#include <vector>
#include <randomx.h>

namespace currency
{
  namespace
  {
    std::mutex g_rx_mutex;
    randomx_flags g_rx_flags = RANDOMX_FLAG_DEFAULT;
    randomx_cache* g_rx_cache = nullptr;
    randomx_dataset* g_rx_dataset = nullptr;
    int g_rx_epoch = -1;
    bool g_rx_mining_mode = false;
    std::vector<randomx_cache*> g_rx_old_caches;
    std::vector<randomx_dataset*> g_rx_old_datasets;

    thread_local randomx_vm* tls_rx_vm = nullptr;
    thread_local int tls_rx_epoch = -1;
    thread_local bool tls_rx_full = false;

    void destroy_tls_vm()
    {
      if (tls_rx_vm)
      {
        randomx_destroy_vm(tls_rx_vm);
        tls_rx_vm = nullptr;
        tls_rx_epoch = -1;
        tls_rx_full = false;
      }
    }

    bool init_dataset_locked()
    {
      if (g_rx_dataset || !g_rx_cache)
        return g_rx_dataset != nullptr;

      LOG_PRINT_YELLOW("RandomX: allocating full dataset for CPU mining (approx. 2 GiB)...", LOG_LEVEL_0);
      g_rx_dataset = randomx_alloc_dataset(g_rx_flags);
      if (!g_rx_dataset)
      {
        LOG_PRINT_RED("RandomX: dataset allocation failed, staying on light mode", LOG_LEVEL_0);
        return false;
      }

      const unsigned long item_count = randomx_dataset_item_count();
      randomx_init_dataset(g_rx_dataset, g_rx_cache, 0, item_count);
      LOG_PRINT_GREEN("RandomX: full dataset ready", LOG_LEVEL_0);
      return true;
    }

    void ensure_epoch_locked(int epoch, const crypto::hash& seed)
    {
      if (g_rx_epoch == epoch && g_rx_cache)
      {
        if (g_rx_mining_mode && !g_rx_dataset)
          init_dataset_locked();
        return;
      }

      if (g_rx_cache)
        g_rx_old_caches.push_back(g_rx_cache);
      if (g_rx_dataset)
      {
        g_rx_old_datasets.push_back(g_rx_dataset);
        g_rx_dataset = nullptr;
      }

      g_rx_flags = randomx_get_flags();
      g_rx_cache = randomx_alloc_cache(g_rx_flags);
      CHECK_AND_ASSERT_THROW_MES(g_rx_cache, "RandomX: failed to allocate cache");
      randomx_init_cache(g_rx_cache, &seed, sizeof(seed));
      g_rx_epoch = epoch;

      if (g_rx_mining_mode)
        init_dataset_locked();
    }

    randomx_vm* ensure_vm_locked(int epoch, const crypto::hash& seed)
    {
      ensure_epoch_locked(epoch, seed);
      const bool want_full = g_rx_dataset != nullptr;
      if (tls_rx_vm && tls_rx_epoch == epoch && tls_rx_full == want_full)
        return tls_rx_vm;

      destroy_tls_vm();
      randomx_flags vm_flags = g_rx_flags;
      if (want_full)
        vm_flags = static_cast<randomx_flags>(vm_flags | RANDOMX_FLAG_FULL_MEM);

      tls_rx_vm = randomx_create_vm(vm_flags, want_full ? nullptr : g_rx_cache, g_rx_dataset);
      CHECK_AND_ASSERT_THROW_MES(tls_rx_vm, "RandomX: failed to create VM");
      tls_rx_epoch = epoch;
      tls_rx_full = want_full;
      return tls_rx_vm;
    }
  }

  int pow_height_to_epoch(uint64_t height)
  {
    return static_cast<int>(height / RANDOMX_EPOCH_LENGTH);
  }

  crypto::hash pow_epoch_to_seed(int epoch)
  {
    uint8_t buf[16] = {};
    memcpy(buf, "PDC-RandomX", 11);
    const uint32_t epoch_le = static_cast<uint32_t>(epoch);
    memcpy(buf + 12, &epoch_le, sizeof(epoch_le));
    return crypto::cn_fast_hash(buf, sizeof(buf));
  }

  void randomx_set_mining_mode(bool enable_full_dataset)
  {
    std::lock_guard<std::mutex> lock(g_rx_mutex);
    g_rx_mining_mode = enable_full_dataset;
    if (!enable_full_dataset && g_rx_dataset)
    {
      g_rx_old_datasets.push_back(g_rx_dataset);
      g_rx_dataset = nullptr;
      destroy_tls_vm();
    }
  }

  crypto::hash get_block_longhash(uint64_t height, const crypto::hash& block_header_hash, uint64_t nonce)
  {
    const int epoch = pow_height_to_epoch(height);
    const crypto::hash seed = pow_epoch_to_seed(epoch);

    uint8_t input[40] = {};
    memcpy(input, &block_header_hash, sizeof(block_header_hash));
    memcpy(input + 32, &nonce, sizeof(nonce));

    randomx_vm* vm = nullptr;
    {
      std::lock_guard<std::mutex> lock(g_rx_mutex);
      vm = ensure_vm_locked(epoch, seed);
    }
    crypto::hash result = null_hash;
    randomx_calculate_hash(vm, input, sizeof(input), &result);
    return result;
  }

  crypto::hash get_block_header_mining_hash(const block& b)
  {
    blobdata bd = get_block_hashing_blob(b);

    access_nonce_in_block_blob(bd) = 0;
    return crypto::cn_fast_hash(bd.data(), bd.size());
  }

  void get_block_longhash(const block& b, crypto::hash& res)
  {
    /*
    RandomX is keyed by epoch seed and hashes (header_hash || nonce_le64).
    Header hash is computed from the block blob with nonce zeroed, same as the
    previous ProgPoW adapter, so stratum/RPC jobs stay (header, seed, nonce).
    Proof-of-stake is unchanged and does not use this function.
    */
    crypto::hash bl_hash = get_block_header_mining_hash(b);
    res = get_block_longhash(get_block_height(b), bl_hash, b.nonce);
  }

  crypto::hash get_block_longhash(const block& b)
  {
    crypto::hash p = null_hash;
    get_block_longhash(b, p);
    return p;
  }
}
