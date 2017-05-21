/*
qgvdial is a cross platform Google Voice Dialer
Copyright (C) 2009-2017 Yuvraaj Kelkar

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA

Contact: yuvraaj@gmail.com
*/

#include "OSDCipher.h"

// 32 character key
//                     "01234567890123456789012345678901"
#define QGV_CIPHER_KEY "__THIS_IS_MY_EXTREMELY_LONG_KEY_"

bool
OsdCipher::_cipher(const QByteArray &byIn, QByteArray &byOut, bool bEncrypt)
{
    // Do I reeealy need to encrypt anything when the app is entirely sandboxed?

    if (bEncrypt) {
        byOut = byIn.toBase64();
    } else {
        byOut = QByteArray::fromBase64(byIn);
    }

    return (true);
}//OsdCipher::cipher
