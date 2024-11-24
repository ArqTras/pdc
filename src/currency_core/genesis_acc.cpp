// Copyright (c) 2014-2018 Zano Project
// Copyright (c) 2014-2018 Zano Project
// Copyright (c) 2014-2018 The Louisdor Project
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "genesis_acc.h"


namespace currency
{
#ifndef TESTNET
  const std::string ggenesis_tx_pub_key_str = "";
  const crypto::public_key ggenesis_tx_pub_key = epee::string_tools::parse_tpod_from_hex_string<crypto::public_key>(ggenesis_tx_pub_key_str);
  const genesis_tx_dictionary_entry ggenesis_dict[2] = {
  //  { 898363347618325980ULL,7 },
  //  { 17472133472787764818ULL,10 }
  };
#else
  const std::string ggenesis_tx_pub_key_str    = "";
  const crypto::public_key ggenesis_tx_pub_key = epee::string_tools::parse_tpod_from_hex_string<crypto::public_key>(ggenesis_tx_pub_key_str);
  const genesis_tx_dictionary_entry ggenesis_dict[5] = {
  //  { 4413532107669521528ULL, 2 },
  //  { 4848259848862559835ULL, 4 },
  //  { 4891306118630423916ULL, 1 },
  //  { 6536034028979999929ULL, 0 },
  //  { 15528122346224653564ULL, 3 }
};
#endif



}
