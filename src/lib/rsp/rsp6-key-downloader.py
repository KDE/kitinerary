#!/usr/bin/env python3
# SPDX-FileCopyrightText: 2023 Volker Krause <vkrause@kde.org>
# SPDX-License-Identifier: LGPL-2.0-or-later

import argparse
import base64
import json
import os
import requests

def hexToBase64(s):
    if len(s) % 2 == 1:
        s = '0' + s
    return base64.b64encode(bytes.fromhex(s)).decode()

parser = argparse.ArgumentParser(description='Download RSP-6 public keys')
parser.add_argument('--output', type=str, required=True, help='Path to which the output should be written')
arguments = parser.parse_args()

os.makedirs(arguments.output, exist_ok = True)

req = requests.get('https://git.eta.st/eta/rsp6-decoder/raw/branch/master/keys.json')
keys = json.loads(req.content)
issuers = []

# remove all existing certs so we clean up revoked/expired ones
for f in os.listdir(arguments.output):
    if f.endswith(".json"):
        os.remove(os.path.join(arguments.output, f))

# create per issuer key files
for issuerId in keys:
    issuerKeys = []
    for key in keys[issuerId]:
        k = {}
        k['n'] = hexToBase64(key['modulus_hex'])
        k['e'] = hexToBase64(key['public_exponent_hex'])
        issuerKeys.append(k)
    keysFile = open(os.path.join(arguments.output, issuerId + '.json'), 'w')
    keysFile.write(json.dumps(issuerKeys))
    issuers.append(issuerId)

# write qrc file
issuers.sort()
qrcFile = open(os.path.join(arguments.output, 'rsp6-keys.qrc'), 'w')
qrcFile.write("""<!--
    SPDX-FileCopyrightText: none
    SPDX-License-Identifier: CC0-1.0
-->
<RCC>
  <qresource prefix="/org.kde.pim/kitinerary/rsp6/keys/">
""")
for issuerId in issuers:
    qrcFile.write(f"    <file>{issuerId}.json</file>\n")
qrcFile.write("""  </qresource>
</RCC>""")
qrcFile.close()
