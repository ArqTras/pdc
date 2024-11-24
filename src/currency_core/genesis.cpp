// Copyright (c) 2014-2018 Zano Project
// Copyright (c) 2014-2018 The Louisdor Project
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "genesis.h"

namespace currency
{
#ifndef TESTNET
  const genesis_tx_raw_data ggenesis_tx_raw = {
    //{
    //  0x179815170a83170c,0xc7e8171ce317da27 },
      //{ 0x17,0x86,0xd6,0x0e,0x0a,0x00,0x00 }
     };
#else
  const genesis_tx_raw_data ggenesis_tx_raw = {
    //{
    //  0xce5017baa917a8f0,0x0a0eefcc17975617},
      //{0x00,0x00}
    };
#endif
}
