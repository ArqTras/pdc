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
    bool g_rx_self_checked = false;
    std::vector<randomx_cache*> g_rx_old_caches;
    std::vector<randomx_dataset*> g_rx_old_datasets;

    thread_local randomx_vm* tls_rx_vm = nullptr;
    thread_local int tls_rx_epoch = -1;
    thread_local bool tls_rx_full = false;

    randomx_flags select_rx_flags()
    {
      randomx_flags flags = randomx_get_flags();
#if defined(__APPLE__) && defined(__aarch64__)
      // RandomARQ's A64 JIT emits into RWX pages without MAP_JIT. On Apple
      // Silicon that leaves a null/non-executable code buffer: the same block
      // hashes to two different PoW values, then JitCompilerA64 SIGSEGVs.
      flags = static_cast<randomx_flags>(flags & ~(RANDOMX_FLAG_JIT | RANDOMX_FLAG_SECURE));
#endif
      return flags;
    }

    void self_check_vm_locked(randomx_vm* vm)
    {
      if (g_rx_self_checked || !vm)
        return;
      crypto::hash first = null_hash;
      crypto::hash second = null_hash;
      uint8_t probe[POW_BLOB_SIZE] = {};
      randomx_calculate_hash(vm, probe, sizeof(probe), &first);
      randomx_calculate_hash(vm, probe, sizeof(probe), &second);
      CHECK_AND_ASSERT_THROW_MES(first == second, "RandomARQ is non-deterministic on this CPU; refusing to verify blocks");
      g_rx_self_checked = true;
      LOG_PRINT_GREEN("RandomARQ self-check OK (flags=" << static_cast<unsigned>(g_rx_flags) << ")", LOG_LEVEL_0);
    }

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

      LOG_PRINT_YELLOW("RandomARQ: allocating full dataset for CPU mining (approx. 2 GiB)...", LOG_LEVEL_0);
      g_rx_dataset = randomx_alloc_dataset(g_rx_flags);
      if (!g_rx_dataset)
      {
        LOG_PRINT_RED("RandomARQ: dataset allocation failed, staying on light mode", LOG_LEVEL_0);
        return false;
      }

      const unsigned long item_count = randomx_dataset_item_count();
      randomx_init_dataset(g_rx_dataset, g_rx_cache, 0, item_count);
      LOG_PRINT_GREEN("RandomARQ: full dataset ready", LOG_LEVEL_0);
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

      g_rx_flags = select_rx_flags();
      g_rx_cache = randomx_alloc_cache(g_rx_flags);
      if (!g_rx_cache && (g_rx_flags & RANDOMX_FLAG_JIT))
      {
        LOG_PRINT_YELLOW("RandomARQ: JIT cache alloc failed, falling back to interpreter", LOG_LEVEL_0);
        g_rx_flags = static_cast<randomx_flags>(g_rx_flags & ~(RANDOMX_FLAG_JIT | RANDOMX_FLAG_SECURE));
        g_rx_cache = randomx_alloc_cache(g_rx_flags);
      }
      CHECK_AND_ASSERT_THROW_MES(g_rx_cache, "RandomARQ: failed to allocate cache");
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
      if (!tls_rx_vm && (vm_flags & RANDOMX_FLAG_JIT))
      {
        LOG_PRINT_YELLOW("RandomARQ: JIT VM failed, recreating cache without JIT", LOG_LEVEL_0);
        if (g_rx_cache)
          g_rx_old_caches.push_back(g_rx_cache);
        g_rx_flags = static_cast<randomx_flags>(g_rx_flags & ~(RANDOMX_FLAG_JIT | RANDOMX_FLAG_SECURE));
        g_rx_cache = randomx_alloc_cache(g_rx_flags);
        CHECK_AND_ASSERT_THROW_MES(g_rx_cache, "RandomARQ: failed to allocate interpreter cache");
        randomx_init_cache(g_rx_cache, &seed, sizeof(seed));
        vm_flags = g_rx_flags;
        if (want_full)
          vm_flags = static_cast<randomx_flags>(vm_flags | RANDOMX_FLAG_FULL_MEM);
        tls_rx_vm = randomx_create_vm(vm_flags, want_full ? nullptr : g_rx_cache, g_rx_dataset);
      }
      CHECK_AND_ASSERT_THROW_MES(tls_rx_vm, "RandomARQ: failed to create VM");
      tls_rx_epoch = epoch;
      tls_rx_full = want_full;
      self_check_vm_locked(tls_rx_vm);
      return tls_rx_vm;
    }
  }

  int pow_height_to_epoch(uint64_t height)
  {
    return static_cast<int>(height / RANDOMX_EPOCH_LENGTH);
  }

  crypto::hash pow_epoch_to_seed(int epoch)
  {
    uint8_t buf[20] = {};
    memcpy(buf, "PDC-RandomARQ", 13);
    const uint32_t epoch_le = static_cast<uint32_t>(epoch);
    memcpy(buf + 16, &epoch_le, sizeof(epoch_le));
    return crypto::cn_fast_hash(buf, sizeof(buf));
  }

  void fill_pow_blob(uint8_t blob[POW_BLOB_SIZE], const crypto::hash& block_header_hash, uint64_t nonce)
  {
    static_assert(POW_BLOB_SIZE == 43, "XMRig rx/arq hashing blob is 43 bytes");
    static_assert(POW_NONCE_OFFSET + sizeof(uint32_t) == POW_BLOB_SIZE, "XMRig nonce is 4 LE bytes at offset 39");
    memset(blob, 0, POW_BLOB_SIZE);
    memcpy(blob, &block_header_hash, sizeof(block_header_hash));
    // RandomARQ / XMRig only search a 32-bit nonce space; ignore high bits.
    const uint32_t nonce32 = static_cast<uint32_t>(nonce & 0xffffffffull);
    memcpy(blob + POW_NONCE_OFFSET, &nonce32, sizeof(nonce32));
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

    uint8_t input[POW_BLOB_SIZE] = {};
    fill_pow_blob(input, block_header_hash, nonce);

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

    set_nonce_in_block_blob(bd, 0);
    return crypto::cn_fast_hash(bd.data(), bd.size());
  }

  void get_block_longhash(const block& b, crypto::hash& res)
  {
    /*
    RandomARQ is keyed by epoch seed and hashes a 43-byte blob matching XMRig rx/arq:
    header_hash (32) || padding (7) || nonce_le32 at offset 39.
    Header hash is computed from the block blob with nonce zeroed.
    Proof-of-stake is unchanged and does not use this function.
    */
    crypto::hash bl_hash = get_block_header_mining_hash(b);
    // Bind block ID to the same 32-bit nonce space XMRig searches.
    res = get_block_longhash(get_block_height(b), bl_hash, b.nonce & 0xffffffffull);
  }

  crypto::hash get_block_longhash(const block& b)
  {
    crypto::hash p = null_hash;
    get_block_longhash(b, p);
    return p;
  }
}
