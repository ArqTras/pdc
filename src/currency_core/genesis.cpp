// Copyright (c) 2014-2018 Zano Project
// Copyright (c) 2014-2018 The Louisdor Project
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "genesis.h"

namespace currency
{
#ifndef TESTNET
	const genesis_tx_raw_data ggenesis_tx_raw = {{
	0x1b03000100000101,0x387e1e4b3ca6fefe,0xbed738a02f773816,0x1c7f66107057e02e,0x003e4b8b3519ebc1,0x033c33c5387b1605,0x19de3dcaf8260427,0x4ce10452416cad3c,0xd72c657fded3ebac,0x364345322813e174,0x4433434632363946,0x3243373938303634,0x3344313545464543,0x4336364141373930,0x020b001542303030},
	{0xaf,0x3b,0x0e,0x0a,0x00,0x00}};

#else
	const genesis_tx_raw_data ggenesis_tx_raw = {{
	0x1b03000100000101,0x387e1e4b3ca6fefe,0xbed738a02f773816,0x1c7f66107057e02e,0x003e4b8b3519ebc1,0x033c33c5387b1605,0x19de3dcaf8260427,0x4ce10452416cad3c,0xd72c657fded3ebac,0x364345322813e174,0x4433434632363946,0x3243373938303634,0x3344313545464543,0x4336364141373930,0x020b001542303030},
	{0xaf,0x3b,0x0e,0x0a,0x00,0x00}};

#endif
}
