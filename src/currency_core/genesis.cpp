// Copyright (c) 2014-2018 Zano Project
// Copyright (c) 2014-2018 The Louisdor Project
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "genesis.h"

namespace currency
{
#ifndef TESTNET
const genesis_tx_raw_data ggenesis_tx_raw = {{
0xf503000100000101,0xbe8c2a387b998f14,0xb032c55a3766dfa8,0x63ff58f07a7b6381,0x00759091b259b686,0xddab2dedd8e81605,0x2b506586693e84eb,0x6f034a8288decd10,0xbae18e123cd82165,0x3643453228133001,0x4433434632363946,0x3243373938303634,0x3344313545464543,0x4336364141373930,0x020b001542303030},
{0xa3,0xb6,0x0e,0x0a,0x00,0x00}};
#else
const genesis_tx_raw_data ggenesis_tx_raw = {{
0xf503000100000101,0xbe8c2a387b998f14,0xb032c55a3766dfa8,0x63ff58f07a7b6381,0x00759091b259b686,0xddab2dedd8e81605,0x2b506586693e84eb,0x6f034a8288decd10,0xbae18e123cd82165,0x3643453228133001,0x4433434632363946,0x3243373938303634,0x3344313545464543,0x4336364141373930,0x020b001542303030},
{0xa3,0xb6,0x0e,0x0a,0x00,0x00}};
#endif
}
