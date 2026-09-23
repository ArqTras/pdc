// Copyright (c) 2014-2018 Zano Project
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#pragma once

#include <typeindex>
#include <unordered_set>
#include <unordered_map>

#include "currency_config.h"
#include "account.h"
#include "include_base_utils.h"

#include "currency_format_utils_abstract.h"
#include "common/crypto_stream_operators.h"
#include "currency_protocol/currency_protocol_defs.h"
#include "crypto/crypto.h"
#include "crypto/hash.h"
#include "difficulty.h"
#include "rpc/core_rpc_server_commands_defs.h"
#include "bc_payments_id_service.h"
#include "bc_attachments_helpers_basic.h"
#include "blockchain_storage_basic.h"

#define CURRENCY_MINER_BLOCK_BLOB_NONCE_OFFSET    1

namespace currency
{
  int pow_height_to_epoch(uint64_t height);
  crypto::hash pow_epoch_to_seed(int epoch);
  void randomx_set_mining_mode(bool enable_full_dataset);
  void fill_pow_blob(uint8_t blob[POW_BLOB_SIZE], const crypto::hash& block_header_hash, uint64_t nonce);

  crypto::hash get_block_header_mining_hash(const block& b);
  crypto::hash get_block_longhash(uint64_t h, const crypto::hash& block_header_hash, uint64_t nonce);
  void get_block_longhash(const block& b, crypto::hash& res);
  crypto::hash get_block_longhash(const block& b);

  inline int ethash_height_to_epoch(uint64_t height) { return pow_height_to_epoch(height); }
  inline crypto::hash ethash_epoch_to_seed(int epoch) { return pow_epoch_to_seed(epoch); }

  inline void set_nonce_in_block_blob(blobdata& bd, uint64_t nonce)
  {
    CHECK_AND_ASSERT_THROW_MES(bd.size() > CURRENCY_MINER_BLOCK_BLOB_NONCE_OFFSET + sizeof(uint64_t) - 1,
      "block hashing blob too short for nonce");
    memcpy(&bd[CURRENCY_MINER_BLOCK_BLOB_NONCE_OFFSET], &nonce, sizeof(nonce));
  }

  inline uint64_t get_nonce_in_block_blob(const blobdata& bd)
  {
    CHECK_AND_ASSERT_THROW_MES(bd.size() > CURRENCY_MINER_BLOCK_BLOB_NONCE_OFFSET + sizeof(uint64_t) - 1,
      "block hashing blob too short for nonce");
    uint64_t nonce = 0;
    memcpy(&nonce, &bd[CURRENCY_MINER_BLOCK_BLOB_NONCE_OFFSET], sizeof(nonce));
    return nonce;
  }
}
