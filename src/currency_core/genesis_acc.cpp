// Copyright (c) 2014-2018 Zano Project
// Copyright (c) 2014-2018 Zano Project
// Copyright (c) 2014-2018 The Louisdor Project
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "genesis_acc.h"


namespace currency
{
#ifndef TESTNET
const std::string ggenesis_tx_pub_key_str = "e8d8ed2dabddeb843e698665502b10cdde88824a036f6521d83c128ee1ba0130";
const crypto::public_key ggenesis_tx_pub_key = epee::string_tools::parse_tpod_from_hex_string<crypto::public_key>(ggenesis_tx_pub_key_str);
extern const genesis_tx_dictionary_entry ggenesis_dict[1];
const genesis_tx_dictionary_entry ggenesis_dict[1] = {
{11874369273325982160ULL,0}
};
#else
const std::string ggenesis_tx_pub_key_str = "e8d8ed2dabddeb843e698665502b10cdde88824a036f6521d83c128ee1ba0130";
const crypto::public_key ggenesis_tx_pub_key = epee::string_tools::parse_tpod_from_hex_string<crypto::public_key>(ggenesis_tx_pub_key_str);
extern const genesis_tx_dictionary_entry ggenesis_dict[1];
const genesis_tx_dictionary_entry ggenesis_dict[1] = {
{11874369273325982160ULL,0}
};
#endif



}
