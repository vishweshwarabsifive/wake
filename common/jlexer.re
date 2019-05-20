/*
 * Copyright 2019 SiFive, Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You should have received a copy of LICENSE.Apache2 along with
 * this software. If not, you may obtain a copy at
 *
 *    https://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "location.h"
#include "lexint.h"
#include "utf8.h"
#include "json5.h"
#include <string.h>

/*!re2c
  re2c:flags:tags = 1;
  re2c:tags:expression = "in.@@";
  re2c:define:YYCTYPE = "unsigned char";
  re2c:flags:8 = 1;
*/

/*!max:re2c*/
static const size_t SIZE = 64 * 1024;

/*!re2c
Ll = [\x61-\x7a\xb5-\xb5\xdf-\xf6\xf8-\u00ff\u0101-\u0101\u0103-\u0103\u0105-\u0105\u0107-\u0107\u0109-\u0109\u010b-\u010b\u010d-\u010d\u010f-\u010f\u0111-\u0111\u0113-\u0113\u0115-\u0115\u0117-\u0117\u0119-\u0119\u011b-\u011b\u011d-\u011d\u011f-\u011f\u0121-\u0121\u0123-\u0123\u0125-\u0125\u0127-\u0127\u0129-\u0129\u012b-\u012b\u012d-\u012d\u012f-\u012f\u0131-\u0131\u0133-\u0133\u0135-\u0135\u0137-\u0138\u013a-\u013a\u013c-\u013c\u013e-\u013e\u0140-\u0140\u0142-\u0142\u0144-\u0144\u0146-\u0146\u0148-\u0149\u014b-\u014b\u014d-\u014d\u014f-\u014f\u0151-\u0151\u0153-\u0153\u0155-\u0155\u0157-\u0157\u0159-\u0159\u015b-\u015b\u015d-\u015d\u015f-\u015f\u0161-\u0161\u0163-\u0163\u0165-\u0165\u0167-\u0167\u0169-\u0169\u016b-\u016b\u016d-\u016d\u016f-\u016f\u0171-\u0171\u0173-\u0173\u0175-\u0175\u0177-\u0177\u017a-\u017a\u017c-\u017c\u017e-\u0180\u0183-\u0183\u0185-\u0185\u0188-\u0188\u018c-\u018d\u0192-\u0192\u0195-\u0195\u0199-\u019b\u019e-\u019e\u01a1-\u01a1\u01a3-\u01a3\u01a5-\u01a5\u01a8-\u01a8\u01aa-\u01ab\u01ad-\u01ad\u01b0-\u01b0\u01b4-\u01b4\u01b6-\u01b6\u01b9-\u01ba\u01bd-\u01bf\u01c6-\u01c6\u01c9-\u01c9\u01cc-\u01cc\u01ce-\u01ce\u01d0-\u01d0\u01d2-\u01d2\u01d4-\u01d4\u01d6-\u01d6\u01d8-\u01d8\u01da-\u01da\u01dc-\u01dd\u01df-\u01df\u01e1-\u01e1\u01e3-\u01e3\u01e5-\u01e5\u01e7-\u01e7\u01e9-\u01e9\u01eb-\u01eb\u01ed-\u01ed\u01ef-\u01f0\u01f3-\u01f3\u01f5-\u01f5\u01f9-\u01f9\u01fb-\u01fb\u01fd-\u01fd\u01ff-\u01ff\u0201-\u0201\u0203-\u0203\u0205-\u0205\u0207-\u0207\u0209-\u0209\u020b-\u020b\u020d-\u020d\u020f-\u020f\u0211-\u0211\u0213-\u0213\u0215-\u0215\u0217-\u0217\u0219-\u0219\u021b-\u021b\u021d-\u021d\u021f-\u021f\u0221-\u0221\u0223-\u0223\u0225-\u0225\u0227-\u0227\u0229-\u0229\u022b-\u022b\u022d-\u022d\u022f-\u022f\u0231-\u0231\u0233-\u0239\u023c-\u023c\u023f-\u0240\u0242-\u0242\u0247-\u0247\u0249-\u0249\u024b-\u024b\u024d-\u024d\u024f-\u0293\u0295-\u02af\u0371-\u0371\u0373-\u0373\u0377-\u0377\u037b-\u037d\u0390-\u0390\u03ac-\u03ce\u03d0-\u03d1\u03d5-\u03d7\u03d9-\u03d9\u03db-\u03db\u03dd-\u03dd\u03df-\u03df\u03e1-\u03e1\u03e3-\u03e3\u03e5-\u03e5\u03e7-\u03e7\u03e9-\u03e9\u03eb-\u03eb\u03ed-\u03ed\u03ef-\u03f3\u03f5-\u03f5\u03f8-\u03f8\u03fb-\u03fc\u0430-\u045f\u0461-\u0461\u0463-\u0463\u0465-\u0465\u0467-\u0467\u0469-\u0469\u046b-\u046b\u046d-\u046d\u046f-\u046f\u0471-\u0471\u0473-\u0473\u0475-\u0475\u0477-\u0477\u0479-\u0479\u047b-\u047b\u047d-\u047d\u047f-\u047f\u0481-\u0481\u048b-\u048b\u048d-\u048d\u048f-\u048f\u0491-\u0491\u0493-\u0493\u0495-\u0495\u0497-\u0497\u0499-\u0499\u049b-\u049b\u049d-\u049d\u049f-\u049f\u04a1-\u04a1\u04a3-\u04a3\u04a5-\u04a5\u04a7-\u04a7\u04a9-\u04a9\u04ab-\u04ab\u04ad-\u04ad\u04af-\u04af\u04b1-\u04b1\u04b3-\u04b3\u04b5-\u04b5\u04b7-\u04b7\u04b9-\u04b9\u04bb-\u04bb\u04bd-\u04bd\u04bf-\u04bf\u04c2-\u04c2\u04c4-\u04c4\u04c6-\u04c6\u04c8-\u04c8\u04ca-\u04ca\u04cc-\u04cc\u04ce-\u04cf\u04d1-\u04d1\u04d3-\u04d3\u04d5-\u04d5\u04d7-\u04d7\u04d9-\u04d9\u04db-\u04db\u04dd-\u04dd\u04df-\u04df\u04e1-\u04e1\u04e3-\u04e3\u04e5-\u04e5\u04e7-\u04e7\u04e9-\u04e9\u04eb-\u04eb\u04ed-\u04ed\u04ef-\u04ef\u04f1-\u04f1\u04f3-\u04f3\u04f5-\u04f5\u04f7-\u04f7\u04f9-\u04f9\u04fb-\u04fb\u04fd-\u04fd\u04ff-\u04ff\u0501-\u0501\u0503-\u0503\u0505-\u0505\u0507-\u0507\u0509-\u0509\u050b-\u050b\u050d-\u050d\u050f-\u050f\u0511-\u0511\u0513-\u0513\u0515-\u0515\u0517-\u0517\u0519-\u0519\u051b-\u051b\u051d-\u051d\u051f-\u051f\u0521-\u0521\u0523-\u0523\u0525-\u0525\u0527-\u0527\u0529-\u0529\u052b-\u052b\u052d-\u052d\u052f-\u052f\u0561-\u0587\u1d00-\u1d2b\u1d6b-\u1d77\u1d79-\u1d9a\u1e01-\u1e01\u1e03-\u1e03\u1e05-\u1e05\u1e07-\u1e07\u1e09-\u1e09\u1e0b-\u1e0b\u1e0d-\u1e0d\u1e0f-\u1e0f\u1e11-\u1e11\u1e13-\u1e13\u1e15-\u1e15\u1e17-\u1e17\u1e19-\u1e19\u1e1b-\u1e1b\u1e1d-\u1e1d\u1e1f-\u1e1f\u1e21-\u1e21\u1e23-\u1e23\u1e25-\u1e25\u1e27-\u1e27\u1e29-\u1e29\u1e2b-\u1e2b\u1e2d-\u1e2d\u1e2f-\u1e2f\u1e31-\u1e31\u1e33-\u1e33\u1e35-\u1e35\u1e37-\u1e37\u1e39-\u1e39\u1e3b-\u1e3b\u1e3d-\u1e3d\u1e3f-\u1e3f\u1e41-\u1e41\u1e43-\u1e43\u1e45-\u1e45\u1e47-\u1e47\u1e49-\u1e49\u1e4b-\u1e4b\u1e4d-\u1e4d\u1e4f-\u1e4f\u1e51-\u1e51\u1e53-\u1e53\u1e55-\u1e55\u1e57-\u1e57\u1e59-\u1e59\u1e5b-\u1e5b\u1e5d-\u1e5d\u1e5f-\u1e5f\u1e61-\u1e61\u1e63-\u1e63\u1e65-\u1e65\u1e67-\u1e67\u1e69-\u1e69\u1e6b-\u1e6b\u1e6d-\u1e6d\u1e6f-\u1e6f\u1e71-\u1e71\u1e73-\u1e73\u1e75-\u1e75\u1e77-\u1e77\u1e79-\u1e79\u1e7b-\u1e7b\u1e7d-\u1e7d\u1e7f-\u1e7f\u1e81-\u1e81\u1e83-\u1e83\u1e85-\u1e85\u1e87-\u1e87\u1e89-\u1e89\u1e8b-\u1e8b\u1e8d-\u1e8d\u1e8f-\u1e8f\u1e91-\u1e91\u1e93-\u1e93\u1e95-\u1e9d\u1e9f-\u1e9f\u1ea1-\u1ea1\u1ea3-\u1ea3\u1ea5-\u1ea5\u1ea7-\u1ea7\u1ea9-\u1ea9\u1eab-\u1eab\u1ead-\u1ead\u1eaf-\u1eaf\u1eb1-\u1eb1\u1eb3-\u1eb3\u1eb5-\u1eb5\u1eb7-\u1eb7\u1eb9-\u1eb9\u1ebb-\u1ebb\u1ebd-\u1ebd\u1ebf-\u1ebf\u1ec1-\u1ec1\u1ec3-\u1ec3\u1ec5-\u1ec5\u1ec7-\u1ec7\u1ec9-\u1ec9\u1ecb-\u1ecb\u1ecd-\u1ecd\u1ecf-\u1ecf\u1ed1-\u1ed1\u1ed3-\u1ed3\u1ed5-\u1ed5\u1ed7-\u1ed7\u1ed9-\u1ed9\u1edb-\u1edb\u1edd-\u1edd\u1edf-\u1edf\u1ee1-\u1ee1\u1ee3-\u1ee3\u1ee5-\u1ee5\u1ee7-\u1ee7\u1ee9-\u1ee9\u1eeb-\u1eeb\u1eed-\u1eed\u1eef-\u1eef\u1ef1-\u1ef1\u1ef3-\u1ef3\u1ef5-\u1ef5\u1ef7-\u1ef7\u1ef9-\u1ef9\u1efb-\u1efb\u1efd-\u1efd\u1eff-\u1f07\u1f10-\u1f15\u1f20-\u1f27\u1f30-\u1f37\u1f40-\u1f45\u1f50-\u1f57\u1f60-\u1f67\u1f70-\u1f7d\u1f80-\u1f87\u1f90-\u1f97\u1fa0-\u1fa7\u1fb0-\u1fb4\u1fb6-\u1fb7\u1fbe-\u1fbe\u1fc2-\u1fc4\u1fc6-\u1fc7\u1fd0-\u1fd3\u1fd6-\u1fd7\u1fe0-\u1fe7\u1ff2-\u1ff4\u1ff6-\u1ff7\u210a-\u210a\u210e-\u210f\u2113-\u2113\u212f-\u212f\u2134-\u2134\u2139-\u2139\u213c-\u213d\u2146-\u2149\u214e-\u214e\u2184-\u2184\u2c30-\u2c5e\u2c61-\u2c61\u2c65-\u2c66\u2c68-\u2c68\u2c6a-\u2c6a\u2c6c-\u2c6c\u2c71-\u2c71\u2c73-\u2c74\u2c76-\u2c7b\u2c81-\u2c81\u2c83-\u2c83\u2c85-\u2c85\u2c87-\u2c87\u2c89-\u2c89\u2c8b-\u2c8b\u2c8d-\u2c8d\u2c8f-\u2c8f\u2c91-\u2c91\u2c93-\u2c93\u2c95-\u2c95\u2c97-\u2c97\u2c99-\u2c99\u2c9b-\u2c9b\u2c9d-\u2c9d\u2c9f-\u2c9f\u2ca1-\u2ca1\u2ca3-\u2ca3\u2ca5-\u2ca5\u2ca7-\u2ca7\u2ca9-\u2ca9\u2cab-\u2cab\u2cad-\u2cad\u2caf-\u2caf\u2cb1-\u2cb1\u2cb3-\u2cb3\u2cb5-\u2cb5\u2cb7-\u2cb7\u2cb9-\u2cb9\u2cbb-\u2cbb\u2cbd-\u2cbd\u2cbf-\u2cbf\u2cc1-\u2cc1\u2cc3-\u2cc3\u2cc5-\u2cc5\u2cc7-\u2cc7\u2cc9-\u2cc9\u2ccb-\u2ccb\u2ccd-\u2ccd\u2ccf-\u2ccf\u2cd1-\u2cd1\u2cd3-\u2cd3\u2cd5-\u2cd5\u2cd7-\u2cd7\u2cd9-\u2cd9\u2cdb-\u2cdb\u2cdd-\u2cdd\u2cdf-\u2cdf\u2ce1-\u2ce1\u2ce3-\u2ce4\u2cec-\u2cec\u2cee-\u2cee\u2cf3-\u2cf3\u2d00-\u2d25\u2d27-\u2d27\u2d2d-\u2d2d\ua641-\ua641\ua643-\ua643\ua645-\ua645\ua647-\ua647\ua649-\ua649\ua64b-\ua64b\ua64d-\ua64d\ua64f-\ua64f\ua651-\ua651\ua653-\ua653\ua655-\ua655\ua657-\ua657\ua659-\ua659\ua65b-\ua65b\ua65d-\ua65d\ua65f-\ua65f\ua661-\ua661\ua663-\ua663\ua665-\ua665\ua667-\ua667\ua669-\ua669\ua66b-\ua66b\ua66d-\ua66d\ua681-\ua681\ua683-\ua683\ua685-\ua685\ua687-\ua687\ua689-\ua689\ua68b-\ua68b\ua68d-\ua68d\ua68f-\ua68f\ua691-\ua691\ua693-\ua693\ua695-\ua695\ua697-\ua697\ua699-\ua699\ua69b-\ua69b\ua723-\ua723\ua725-\ua725\ua727-\ua727\ua729-\ua729\ua72b-\ua72b\ua72d-\ua72d\ua72f-\ua731\ua733-\ua733\ua735-\ua735\ua737-\ua737\ua739-\ua739\ua73b-\ua73b\ua73d-\ua73d\ua73f-\ua73f\ua741-\ua741\ua743-\ua743\ua745-\ua745\ua747-\ua747\ua749-\ua749\ua74b-\ua74b\ua74d-\ua74d\ua74f-\ua74f\ua751-\ua751\ua753-\ua753\ua755-\ua755\ua757-\ua757\ua759-\ua759\ua75b-\ua75b\ua75d-\ua75d\ua75f-\ua75f\ua761-\ua761\ua763-\ua763\ua765-\ua765\ua767-\ua767\ua769-\ua769\ua76b-\ua76b\ua76d-\ua76d\ua76f-\ua76f\ua771-\ua778\ua77a-\ua77a\ua77c-\ua77c\ua77f-\ua77f\ua781-\ua781\ua783-\ua783\ua785-\ua785\ua787-\ua787\ua78c-\ua78c\ua78e-\ua78e\ua791-\ua791\ua793-\ua795\ua797-\ua797\ua799-\ua799\ua79b-\ua79b\ua79d-\ua79d\ua79f-\ua79f\ua7a1-\ua7a1\ua7a3-\ua7a3\ua7a5-\ua7a5\ua7a7-\ua7a7\ua7a9-\ua7a9\ua7fa-\ua7fa\uab30-\uab5a\uab64-\uab65\ufb00-\ufb06\ufb13-\ufb17\uff41-\uff5a\U00010428-\U0001044f\U000118c0-\U000118df\U0001d41a-\U0001d433\U0001d44e-\U0001d454\U0001d456-\U0001d467\U0001d482-\U0001d49b\U0001d4b6-\U0001d4b9\U0001d4bb-\U0001d4bb\U0001d4bd-\U0001d4c3\U0001d4c5-\U0001d4cf\U0001d4ea-\U0001d503\U0001d51e-\U0001d537\U0001d552-\U0001d56b\U0001d586-\U0001d59f\U0001d5ba-\U0001d5d3\U0001d5ee-\U0001d607\U0001d622-\U0001d63b\U0001d656-\U0001d66f\U0001d68a-\U0001d6a5\U0001d6c2-\U0001d6da\U0001d6dc-\U0001d6e1\U0001d6fc-\U0001d714\U0001d716-\U0001d71b\U0001d736-\U0001d74e\U0001d750-\U0001d755\U0001d770-\U0001d788\U0001d78a-\U0001d78f\U0001d7aa-\U0001d7c2\U0001d7c4-\U0001d7c9\U0001d7cb-\U0001d7cb];
Lu = [\x41-\x5a\xc0-\xd6\xd8-\xde\u0100-\u0100\u0102-\u0102\u0104-\u0104\u0106-\u0106\u0108-\u0108\u010a-\u010a\u010c-\u010c\u010e-\u010e\u0110-\u0110\u0112-\u0112\u0114-\u0114\u0116-\u0116\u0118-\u0118\u011a-\u011a\u011c-\u011c\u011e-\u011e\u0120-\u0120\u0122-\u0122\u0124-\u0124\u0126-\u0126\u0128-\u0128\u012a-\u012a\u012c-\u012c\u012e-\u012e\u0130-\u0130\u0132-\u0132\u0134-\u0134\u0136-\u0136\u0139-\u0139\u013b-\u013b\u013d-\u013d\u013f-\u013f\u0141-\u0141\u0143-\u0143\u0145-\u0145\u0147-\u0147\u014a-\u014a\u014c-\u014c\u014e-\u014e\u0150-\u0150\u0152-\u0152\u0154-\u0154\u0156-\u0156\u0158-\u0158\u015a-\u015a\u015c-\u015c\u015e-\u015e\u0160-\u0160\u0162-\u0162\u0164-\u0164\u0166-\u0166\u0168-\u0168\u016a-\u016a\u016c-\u016c\u016e-\u016e\u0170-\u0170\u0172-\u0172\u0174-\u0174\u0176-\u0176\u0178-\u0179\u017b-\u017b\u017d-\u017d\u0181-\u0182\u0184-\u0184\u0186-\u0187\u0189-\u018b\u018e-\u0191\u0193-\u0194\u0196-\u0198\u019c-\u019d\u019f-\u01a0\u01a2-\u01a2\u01a4-\u01a4\u01a6-\u01a7\u01a9-\u01a9\u01ac-\u01ac\u01ae-\u01af\u01b1-\u01b3\u01b5-\u01b5\u01b7-\u01b8\u01bc-\u01bc\u01c4-\u01c4\u01c7-\u01c7\u01ca-\u01ca\u01cd-\u01cd\u01cf-\u01cf\u01d1-\u01d1\u01d3-\u01d3\u01d5-\u01d5\u01d7-\u01d7\u01d9-\u01d9\u01db-\u01db\u01de-\u01de\u01e0-\u01e0\u01e2-\u01e2\u01e4-\u01e4\u01e6-\u01e6\u01e8-\u01e8\u01ea-\u01ea\u01ec-\u01ec\u01ee-\u01ee\u01f1-\u01f1\u01f4-\u01f4\u01f6-\u01f8\u01fa-\u01fa\u01fc-\u01fc\u01fe-\u01fe\u0200-\u0200\u0202-\u0202\u0204-\u0204\u0206-\u0206\u0208-\u0208\u020a-\u020a\u020c-\u020c\u020e-\u020e\u0210-\u0210\u0212-\u0212\u0214-\u0214\u0216-\u0216\u0218-\u0218\u021a-\u021a\u021c-\u021c\u021e-\u021e\u0220-\u0220\u0222-\u0222\u0224-\u0224\u0226-\u0226\u0228-\u0228\u022a-\u022a\u022c-\u022c\u022e-\u022e\u0230-\u0230\u0232-\u0232\u023a-\u023b\u023d-\u023e\u0241-\u0241\u0243-\u0246\u0248-\u0248\u024a-\u024a\u024c-\u024c\u024e-\u024e\u0370-\u0370\u0372-\u0372\u0376-\u0376\u037f-\u037f\u0386-\u0386\u0388-\u038a\u038c-\u038c\u038e-\u038f\u0391-\u03a1\u03a3-\u03ab\u03cf-\u03cf\u03d2-\u03d4\u03d8-\u03d8\u03da-\u03da\u03dc-\u03dc\u03de-\u03de\u03e0-\u03e0\u03e2-\u03e2\u03e4-\u03e4\u03e6-\u03e6\u03e8-\u03e8\u03ea-\u03ea\u03ec-\u03ec\u03ee-\u03ee\u03f4-\u03f4\u03f7-\u03f7\u03f9-\u03fa\u03fd-\u042f\u0460-\u0460\u0462-\u0462\u0464-\u0464\u0466-\u0466\u0468-\u0468\u046a-\u046a\u046c-\u046c\u046e-\u046e\u0470-\u0470\u0472-\u0472\u0474-\u0474\u0476-\u0476\u0478-\u0478\u047a-\u047a\u047c-\u047c\u047e-\u047e\u0480-\u0480\u048a-\u048a\u048c-\u048c\u048e-\u048e\u0490-\u0490\u0492-\u0492\u0494-\u0494\u0496-\u0496\u0498-\u0498\u049a-\u049a\u049c-\u049c\u049e-\u049e\u04a0-\u04a0\u04a2-\u04a2\u04a4-\u04a4\u04a6-\u04a6\u04a8-\u04a8\u04aa-\u04aa\u04ac-\u04ac\u04ae-\u04ae\u04b0-\u04b0\u04b2-\u04b2\u04b4-\u04b4\u04b6-\u04b6\u04b8-\u04b8\u04ba-\u04ba\u04bc-\u04bc\u04be-\u04be\u04c0-\u04c1\u04c3-\u04c3\u04c5-\u04c5\u04c7-\u04c7\u04c9-\u04c9\u04cb-\u04cb\u04cd-\u04cd\u04d0-\u04d0\u04d2-\u04d2\u04d4-\u04d4\u04d6-\u04d6\u04d8-\u04d8\u04da-\u04da\u04dc-\u04dc\u04de-\u04de\u04e0-\u04e0\u04e2-\u04e2\u04e4-\u04e4\u04e6-\u04e6\u04e8-\u04e8\u04ea-\u04ea\u04ec-\u04ec\u04ee-\u04ee\u04f0-\u04f0\u04f2-\u04f2\u04f4-\u04f4\u04f6-\u04f6\u04f8-\u04f8\u04fa-\u04fa\u04fc-\u04fc\u04fe-\u04fe\u0500-\u0500\u0502-\u0502\u0504-\u0504\u0506-\u0506\u0508-\u0508\u050a-\u050a\u050c-\u050c\u050e-\u050e\u0510-\u0510\u0512-\u0512\u0514-\u0514\u0516-\u0516\u0518-\u0518\u051a-\u051a\u051c-\u051c\u051e-\u051e\u0520-\u0520\u0522-\u0522\u0524-\u0524\u0526-\u0526\u0528-\u0528\u052a-\u052a\u052c-\u052c\u052e-\u052e\u0531-\u0556\u10a0-\u10c5\u10c7-\u10c7\u10cd-\u10cd\u1e00-\u1e00\u1e02-\u1e02\u1e04-\u1e04\u1e06-\u1e06\u1e08-\u1e08\u1e0a-\u1e0a\u1e0c-\u1e0c\u1e0e-\u1e0e\u1e10-\u1e10\u1e12-\u1e12\u1e14-\u1e14\u1e16-\u1e16\u1e18-\u1e18\u1e1a-\u1e1a\u1e1c-\u1e1c\u1e1e-\u1e1e\u1e20-\u1e20\u1e22-\u1e22\u1e24-\u1e24\u1e26-\u1e26\u1e28-\u1e28\u1e2a-\u1e2a\u1e2c-\u1e2c\u1e2e-\u1e2e\u1e30-\u1e30\u1e32-\u1e32\u1e34-\u1e34\u1e36-\u1e36\u1e38-\u1e38\u1e3a-\u1e3a\u1e3c-\u1e3c\u1e3e-\u1e3e\u1e40-\u1e40\u1e42-\u1e42\u1e44-\u1e44\u1e46-\u1e46\u1e48-\u1e48\u1e4a-\u1e4a\u1e4c-\u1e4c\u1e4e-\u1e4e\u1e50-\u1e50\u1e52-\u1e52\u1e54-\u1e54\u1e56-\u1e56\u1e58-\u1e58\u1e5a-\u1e5a\u1e5c-\u1e5c\u1e5e-\u1e5e\u1e60-\u1e60\u1e62-\u1e62\u1e64-\u1e64\u1e66-\u1e66\u1e68-\u1e68\u1e6a-\u1e6a\u1e6c-\u1e6c\u1e6e-\u1e6e\u1e70-\u1e70\u1e72-\u1e72\u1e74-\u1e74\u1e76-\u1e76\u1e78-\u1e78\u1e7a-\u1e7a\u1e7c-\u1e7c\u1e7e-\u1e7e\u1e80-\u1e80\u1e82-\u1e82\u1e84-\u1e84\u1e86-\u1e86\u1e88-\u1e88\u1e8a-\u1e8a\u1e8c-\u1e8c\u1e8e-\u1e8e\u1e90-\u1e90\u1e92-\u1e92\u1e94-\u1e94\u1e9e-\u1e9e\u1ea0-\u1ea0\u1ea2-\u1ea2\u1ea4-\u1ea4\u1ea6-\u1ea6\u1ea8-\u1ea8\u1eaa-\u1eaa\u1eac-\u1eac\u1eae-\u1eae\u1eb0-\u1eb0\u1eb2-\u1eb2\u1eb4-\u1eb4\u1eb6-\u1eb6\u1eb8-\u1eb8\u1eba-\u1eba\u1ebc-\u1ebc\u1ebe-\u1ebe\u1ec0-\u1ec0\u1ec2-\u1ec2\u1ec4-\u1ec4\u1ec6-\u1ec6\u1ec8-\u1ec8\u1eca-\u1eca\u1ecc-\u1ecc\u1ece-\u1ece\u1ed0-\u1ed0\u1ed2-\u1ed2\u1ed4-\u1ed4\u1ed6-\u1ed6\u1ed8-\u1ed8\u1eda-\u1eda\u1edc-\u1edc\u1ede-\u1ede\u1ee0-\u1ee0\u1ee2-\u1ee2\u1ee4-\u1ee4\u1ee6-\u1ee6\u1ee8-\u1ee8\u1eea-\u1eea\u1eec-\u1eec\u1eee-\u1eee\u1ef0-\u1ef0\u1ef2-\u1ef2\u1ef4-\u1ef4\u1ef6-\u1ef6\u1ef8-\u1ef8\u1efa-\u1efa\u1efc-\u1efc\u1efe-\u1efe\u1f08-\u1f0f\u1f18-\u1f1d\u1f28-\u1f2f\u1f38-\u1f3f\u1f48-\u1f4d\u1f59-\u1f59\u1f5b-\u1f5b\u1f5d-\u1f5d\u1f5f-\u1f5f\u1f68-\u1f6f\u1fb8-\u1fbb\u1fc8-\u1fcb\u1fd8-\u1fdb\u1fe8-\u1fec\u1ff8-\u1ffb\u2102-\u2102\u2107-\u2107\u210b-\u210d\u2110-\u2112\u2115-\u2115\u2119-\u211d\u2124-\u2124\u2126-\u2126\u2128-\u2128\u212a-\u212d\u2130-\u2133\u213e-\u213f\u2145-\u2145\u2183-\u2183\u2c00-\u2c2e\u2c60-\u2c60\u2c62-\u2c64\u2c67-\u2c67\u2c69-\u2c69\u2c6b-\u2c6b\u2c6d-\u2c70\u2c72-\u2c72\u2c75-\u2c75\u2c7e-\u2c80\u2c82-\u2c82\u2c84-\u2c84\u2c86-\u2c86\u2c88-\u2c88\u2c8a-\u2c8a\u2c8c-\u2c8c\u2c8e-\u2c8e\u2c90-\u2c90\u2c92-\u2c92\u2c94-\u2c94\u2c96-\u2c96\u2c98-\u2c98\u2c9a-\u2c9a\u2c9c-\u2c9c\u2c9e-\u2c9e\u2ca0-\u2ca0\u2ca2-\u2ca2\u2ca4-\u2ca4\u2ca6-\u2ca6\u2ca8-\u2ca8\u2caa-\u2caa\u2cac-\u2cac\u2cae-\u2cae\u2cb0-\u2cb0\u2cb2-\u2cb2\u2cb4-\u2cb4\u2cb6-\u2cb6\u2cb8-\u2cb8\u2cba-\u2cba\u2cbc-\u2cbc\u2cbe-\u2cbe\u2cc0-\u2cc0\u2cc2-\u2cc2\u2cc4-\u2cc4\u2cc6-\u2cc6\u2cc8-\u2cc8\u2cca-\u2cca\u2ccc-\u2ccc\u2cce-\u2cce\u2cd0-\u2cd0\u2cd2-\u2cd2\u2cd4-\u2cd4\u2cd6-\u2cd6\u2cd8-\u2cd8\u2cda-\u2cda\u2cdc-\u2cdc\u2cde-\u2cde\u2ce0-\u2ce0\u2ce2-\u2ce2\u2ceb-\u2ceb\u2ced-\u2ced\u2cf2-\u2cf2\ua640-\ua640\ua642-\ua642\ua644-\ua644\ua646-\ua646\ua648-\ua648\ua64a-\ua64a\ua64c-\ua64c\ua64e-\ua64e\ua650-\ua650\ua652-\ua652\ua654-\ua654\ua656-\ua656\ua658-\ua658\ua65a-\ua65a\ua65c-\ua65c\ua65e-\ua65e\ua660-\ua660\ua662-\ua662\ua664-\ua664\ua666-\ua666\ua668-\ua668\ua66a-\ua66a\ua66c-\ua66c\ua680-\ua680\ua682-\ua682\ua684-\ua684\ua686-\ua686\ua688-\ua688\ua68a-\ua68a\ua68c-\ua68c\ua68e-\ua68e\ua690-\ua690\ua692-\ua692\ua694-\ua694\ua696-\ua696\ua698-\ua698\ua69a-\ua69a\ua722-\ua722\ua724-\ua724\ua726-\ua726\ua728-\ua728\ua72a-\ua72a\ua72c-\ua72c\ua72e-\ua72e\ua732-\ua732\ua734-\ua734\ua736-\ua736\ua738-\ua738\ua73a-\ua73a\ua73c-\ua73c\ua73e-\ua73e\ua740-\ua740\ua742-\ua742\ua744-\ua744\ua746-\ua746\ua748-\ua748\ua74a-\ua74a\ua74c-\ua74c\ua74e-\ua74e\ua750-\ua750\ua752-\ua752\ua754-\ua754\ua756-\ua756\ua758-\ua758\ua75a-\ua75a\ua75c-\ua75c\ua75e-\ua75e\ua760-\ua760\ua762-\ua762\ua764-\ua764\ua766-\ua766\ua768-\ua768\ua76a-\ua76a\ua76c-\ua76c\ua76e-\ua76e\ua779-\ua779\ua77b-\ua77b\ua77d-\ua77e\ua780-\ua780\ua782-\ua782\ua784-\ua784\ua786-\ua786\ua78b-\ua78b\ua78d-\ua78d\ua790-\ua790\ua792-\ua792\ua796-\ua796\ua798-\ua798\ua79a-\ua79a\ua79c-\ua79c\ua79e-\ua79e\ua7a0-\ua7a0\ua7a2-\ua7a2\ua7a4-\ua7a4\ua7a6-\ua7a6\ua7a8-\ua7a8\ua7aa-\ua7ad\ua7b0-\ua7b1\uff21-\uff3a\U00010400-\U00010427\U000118a0-\U000118bf\U0001d400-\U0001d419\U0001d434-\U0001d44d\U0001d468-\U0001d481\U0001d49c-\U0001d49c\U0001d49e-\U0001d49f\U0001d4a2-\U0001d4a2\U0001d4a5-\U0001d4a6\U0001d4a9-\U0001d4ac\U0001d4ae-\U0001d4b5\U0001d4d0-\U0001d4e9\U0001d504-\U0001d505\U0001d507-\U0001d50a\U0001d50d-\U0001d514\U0001d516-\U0001d51c\U0001d538-\U0001d539\U0001d53b-\U0001d53e\U0001d540-\U0001d544\U0001d546-\U0001d546\U0001d54a-\U0001d550\U0001d56c-\U0001d585\U0001d5a0-\U0001d5b9\U0001d5d4-\U0001d5ed\U0001d608-\U0001d621\U0001d63c-\U0001d655\U0001d670-\U0001d689\U0001d6a8-\U0001d6c0\U0001d6e2-\U0001d6fa\U0001d71c-\U0001d734\U0001d756-\U0001d76e\U0001d790-\U0001d7a8\U0001d7ca-\U0001d7ca];
Lt = [\u01c5-\u01c5\u01c8-\u01c8\u01cb-\u01cb\u01f2-\u01f2\u1f88-\u1f8f\u1f98-\u1f9f\u1fa8-\u1faf\u1fbc-\u1fbc\u1fcc-\u1fcc\u1ffc-\u1ffc];
Lm = [\u02b0-\u02c1\u02c6-\u02d1\u02e0-\u02e4\u02ec-\u02ec\u02ee-\u02ee\u0374-\u0374\u037a-\u037a\u0559-\u0559\u0640-\u0640\u06e5-\u06e6\u07f4-\u07f5\u07fa-\u07fa\u081a-\u081a\u0824-\u0824\u0828-\u0828\u0971-\u0971\u0e46-\u0e46\u0ec6-\u0ec6\u10fc-\u10fc\u17d7-\u17d7\u1843-\u1843\u1aa7-\u1aa7\u1c78-\u1c7d\u1d2c-\u1d6a\u1d78-\u1d78\u1d9b-\u1dbf\u2071-\u2071\u207f-\u207f\u2090-\u209c\u2c7c-\u2c7d\u2d6f-\u2d6f\u2e2f-\u2e2f\u3005-\u3005\u3031-\u3035\u303b-\u303b\u309d-\u309e\u30fc-\u30fe\ua015-\ua015\ua4f8-\ua4fd\ua60c-\ua60c\ua67f-\ua67f\ua69c-\ua69d\ua717-\ua71f\ua770-\ua770\ua788-\ua788\ua7f8-\ua7f9\ua9cf-\ua9cf\ua9e6-\ua9e6\uaa70-\uaa70\uaadd-\uaadd\uaaf3-\uaaf4\uab5c-\uab5f\uff70-\uff70\uff9e-\uff9f\U00016b40-\U00016b43\U00016f93-\U00016f9f];
Lo = [\xaa-\xaa\xba-\xba\u01bb-\u01bb\u01c0-\u01c3\u0294-\u0294\u05d0-\u05ea\u05f0-\u05f2\u0620-\u063f\u0641-\u064a\u066e-\u066f\u0671-\u06d3\u06d5-\u06d5\u06ee-\u06ef\u06fa-\u06fc\u06ff-\u06ff\u0710-\u0710\u0712-\u072f\u074d-\u07a5\u07b1-\u07b1\u07ca-\u07ea\u0800-\u0815\u0840-\u0858\u08a0-\u08b2\u0904-\u0939\u093d-\u093d\u0950-\u0950\u0958-\u0961\u0972-\u0980\u0985-\u098c\u098f-\u0990\u0993-\u09a8\u09aa-\u09b0\u09b2-\u09b2\u09b6-\u09b9\u09bd-\u09bd\u09ce-\u09ce\u09dc-\u09dd\u09df-\u09e1\u09f0-\u09f1\u0a05-\u0a0a\u0a0f-\u0a10\u0a13-\u0a28\u0a2a-\u0a30\u0a32-\u0a33\u0a35-\u0a36\u0a38-\u0a39\u0a59-\u0a5c\u0a5e-\u0a5e\u0a72-\u0a74\u0a85-\u0a8d\u0a8f-\u0a91\u0a93-\u0aa8\u0aaa-\u0ab0\u0ab2-\u0ab3\u0ab5-\u0ab9\u0abd-\u0abd\u0ad0-\u0ad0\u0ae0-\u0ae1\u0b05-\u0b0c\u0b0f-\u0b10\u0b13-\u0b28\u0b2a-\u0b30\u0b32-\u0b33\u0b35-\u0b39\u0b3d-\u0b3d\u0b5c-\u0b5d\u0b5f-\u0b61\u0b71-\u0b71\u0b83-\u0b83\u0b85-\u0b8a\u0b8e-\u0b90\u0b92-\u0b95\u0b99-\u0b9a\u0b9c-\u0b9c\u0b9e-\u0b9f\u0ba3-\u0ba4\u0ba8-\u0baa\u0bae-\u0bb9\u0bd0-\u0bd0\u0c05-\u0c0c\u0c0e-\u0c10\u0c12-\u0c28\u0c2a-\u0c39\u0c3d-\u0c3d\u0c58-\u0c59\u0c60-\u0c61\u0c85-\u0c8c\u0c8e-\u0c90\u0c92-\u0ca8\u0caa-\u0cb3\u0cb5-\u0cb9\u0cbd-\u0cbd\u0cde-\u0cde\u0ce0-\u0ce1\u0cf1-\u0cf2\u0d05-\u0d0c\u0d0e-\u0d10\u0d12-\u0d3a\u0d3d-\u0d3d\u0d4e-\u0d4e\u0d60-\u0d61\u0d7a-\u0d7f\u0d85-\u0d96\u0d9a-\u0db1\u0db3-\u0dbb\u0dbd-\u0dbd\u0dc0-\u0dc6\u0e01-\u0e30\u0e32-\u0e33\u0e40-\u0e45\u0e81-\u0e82\u0e84-\u0e84\u0e87-\u0e88\u0e8a-\u0e8a\u0e8d-\u0e8d\u0e94-\u0e97\u0e99-\u0e9f\u0ea1-\u0ea3\u0ea5-\u0ea5\u0ea7-\u0ea7\u0eaa-\u0eab\u0ead-\u0eb0\u0eb2-\u0eb3\u0ebd-\u0ebd\u0ec0-\u0ec4\u0edc-\u0edf\u0f00-\u0f00\u0f40-\u0f47\u0f49-\u0f6c\u0f88-\u0f8c\u1000-\u102a\u103f-\u103f\u1050-\u1055\u105a-\u105d\u1061-\u1061\u1065-\u1066\u106e-\u1070\u1075-\u1081\u108e-\u108e\u10d0-\u10fa\u10fd-\u1248\u124a-\u124d\u1250-\u1256\u1258-\u1258\u125a-\u125d\u1260-\u1288\u128a-\u128d\u1290-\u12b0\u12b2-\u12b5\u12b8-\u12be\u12c0-\u12c0\u12c2-\u12c5\u12c8-\u12d6\u12d8-\u1310\u1312-\u1315\u1318-\u135a\u1380-\u138f\u13a0-\u13f4\u1401-\u166c\u166f-\u167f\u1681-\u169a\u16a0-\u16ea\u16f1-\u16f8\u1700-\u170c\u170e-\u1711\u1720-\u1731\u1740-\u1751\u1760-\u176c\u176e-\u1770\u1780-\u17b3\u17dc-\u17dc\u1820-\u1842\u1844-\u1877\u1880-\u18a8\u18aa-\u18aa\u18b0-\u18f5\u1900-\u191e\u1950-\u196d\u1970-\u1974\u1980-\u19ab\u19c1-\u19c7\u1a00-\u1a16\u1a20-\u1a54\u1b05-\u1b33\u1b45-\u1b4b\u1b83-\u1ba0\u1bae-\u1baf\u1bba-\u1be5\u1c00-\u1c23\u1c4d-\u1c4f\u1c5a-\u1c77\u1ce9-\u1cec\u1cee-\u1cf1\u1cf5-\u1cf6\u2135-\u2138\u2d30-\u2d67\u2d80-\u2d96\u2da0-\u2da6\u2da8-\u2dae\u2db0-\u2db6\u2db8-\u2dbe\u2dc0-\u2dc6\u2dc8-\u2dce\u2dd0-\u2dd6\u2dd8-\u2dde\u3006-\u3006\u303c-\u303c\u3041-\u3096\u309f-\u309f\u30a1-\u30fa\u30ff-\u30ff\u3105-\u312d\u3131-\u318e\u31a0-\u31ba\u31f0-\u31ff\u3400-\u4db5\u4e00-\u9fcc\ua000-\ua014\ua016-\ua48c\ua4d0-\ua4f7\ua500-\ua60b\ua610-\ua61f\ua62a-\ua62b\ua66e-\ua66e\ua6a0-\ua6e5\ua7f7-\ua7f7\ua7fb-\ua801\ua803-\ua805\ua807-\ua80a\ua80c-\ua822\ua840-\ua873\ua882-\ua8b3\ua8f2-\ua8f7\ua8fb-\ua8fb\ua90a-\ua925\ua930-\ua946\ua960-\ua97c\ua984-\ua9b2\ua9e0-\ua9e4\ua9e7-\ua9ef\ua9fa-\ua9fe\uaa00-\uaa28\uaa40-\uaa42\uaa44-\uaa4b\uaa60-\uaa6f\uaa71-\uaa76\uaa7a-\uaa7a\uaa7e-\uaaaf\uaab1-\uaab1\uaab5-\uaab6\uaab9-\uaabd\uaac0-\uaac0\uaac2-\uaac2\uaadb-\uaadc\uaae0-\uaaea\uaaf2-\uaaf2\uab01-\uab06\uab09-\uab0e\uab11-\uab16\uab20-\uab26\uab28-\uab2e\uabc0-\uabe2\uac00-\ud7a3\ud7b0-\ud7c6\ud7cb-\ud7fb\uf900-\ufa6d\ufa70-\ufad9\ufb1d-\ufb1d\ufb1f-\ufb28\ufb2a-\ufb36\ufb38-\ufb3c\ufb3e-\ufb3e\ufb40-\ufb41\ufb43-\ufb44\ufb46-\ufbb1\ufbd3-\ufd3d\ufd50-\ufd8f\ufd92-\ufdc7\ufdf0-\ufdfb\ufe70-\ufe74\ufe76-\ufefc\uff66-\uff6f\uff71-\uff9d\uffa0-\uffbe\uffc2-\uffc7\uffca-\uffcf\uffd2-\uffd7\uffda-\uffdc\U00010000-\U0001000b\U0001000d-\U00010026\U00010028-\U0001003a\U0001003c-\U0001003d\U0001003f-\U0001004d\U00010050-\U0001005d\U00010080-\U000100fa\U00010280-\U0001029c\U000102a0-\U000102d0\U00010300-\U0001031f\U00010330-\U00010340\U00010342-\U00010349\U00010350-\U00010375\U00010380-\U0001039d\U000103a0-\U000103c3\U000103c8-\U000103cf\U00010450-\U0001049d\U00010500-\U00010527\U00010530-\U00010563\U00010600-\U00010736\U00010740-\U00010755\U00010760-\U00010767\U00010800-\U00010805\U00010808-\U00010808\U0001080a-\U00010835\U00010837-\U00010838\U0001083c-\U0001083c\U0001083f-\U00010855\U00010860-\U00010876\U00010880-\U0001089e\U00010900-\U00010915\U00010920-\U00010939\U00010980-\U000109b7\U000109be-\U000109bf\U00010a00-\U00010a00\U00010a10-\U00010a13\U00010a15-\U00010a17\U00010a19-\U00010a33\U00010a60-\U00010a7c\U00010a80-\U00010a9c\U00010ac0-\U00010ac7\U00010ac9-\U00010ae4\U00010b00-\U00010b35\U00010b40-\U00010b55\U00010b60-\U00010b72\U00010b80-\U00010b91\U00010c00-\U00010c48\U00011003-\U00011037\U00011083-\U000110af\U000110d0-\U000110e8\U00011103-\U00011126\U00011150-\U00011172\U00011176-\U00011176\U00011183-\U000111b2\U000111c1-\U000111c4\U000111da-\U000111da\U00011200-\U00011211\U00011213-\U0001122b\U000112b0-\U000112de\U00011305-\U0001130c\U0001130f-\U00011310\U00011313-\U00011328\U0001132a-\U00011330\U00011332-\U00011333\U00011335-\U00011339\U0001133d-\U0001133d\U0001135d-\U00011361\U00011480-\U000114af\U000114c4-\U000114c5\U000114c7-\U000114c7\U00011580-\U000115ae\U00011600-\U0001162f\U00011644-\U00011644\U00011680-\U000116aa\U000118ff-\U000118ff\U00011ac0-\U00011af8\U00012000-\U00012398\U00013000-\U0001342e\U00016800-\U00016a38\U00016a40-\U00016a5e\U00016ad0-\U00016aed\U00016b00-\U00016b2f\U00016b63-\U00016b77\U00016b7d-\U00016b8f\U00016f00-\U00016f44\U00016f50-\U00016f50\U0001b000-\U0001b001\U0001bc00-\U0001bc6a\U0001bc70-\U0001bc7c\U0001bc80-\U0001bc88\U0001bc90-\U0001bc99\U0001e800-\U0001e8c4\U0001ee00-\U0001ee03\U0001ee05-\U0001ee1f\U0001ee21-\U0001ee22\U0001ee24-\U0001ee24\U0001ee27-\U0001ee27\U0001ee29-\U0001ee32\U0001ee34-\U0001ee37\U0001ee39-\U0001ee39\U0001ee3b-\U0001ee3b\U0001ee42-\U0001ee42\U0001ee47-\U0001ee47\U0001ee49-\U0001ee49\U0001ee4b-\U0001ee4b\U0001ee4d-\U0001ee4f\U0001ee51-\U0001ee52\U0001ee54-\U0001ee54\U0001ee57-\U0001ee57\U0001ee59-\U0001ee59\U0001ee5b-\U0001ee5b\U0001ee5d-\U0001ee5d\U0001ee5f-\U0001ee5f\U0001ee61-\U0001ee62\U0001ee64-\U0001ee64\U0001ee67-\U0001ee6a\U0001ee6c-\U0001ee72\U0001ee74-\U0001ee77\U0001ee79-\U0001ee7c\U0001ee7e-\U0001ee7e\U0001ee80-\U0001ee89\U0001ee8b-\U0001ee9b\U0001eea1-\U0001eea3\U0001eea5-\U0001eea9\U0001eeab-\U0001eebb\U00020000-\U0002a6d6\U0002a700-\U0002b734\U0002b740-\U0002b81d\U0002f800-\U0002fa1d];
Mn = [\u0300-\u036f\u0483-\u0487\u0591-\u05bd\u05bf-\u05bf\u05c1-\u05c2\u05c4-\u05c5\u05c7-\u05c7\u0610-\u061a\u064b-\u065f\u0670-\u0670\u06d6-\u06dc\u06df-\u06e4\u06e7-\u06e8\u06ea-\u06ed\u0711-\u0711\u0730-\u074a\u07a6-\u07b0\u07eb-\u07f3\u0816-\u0819\u081b-\u0823\u0825-\u0827\u0829-\u082d\u0859-\u085b\u08e4-\u0902\u093a-\u093a\u093c-\u093c\u0941-\u0948\u094d-\u094d\u0951-\u0957\u0962-\u0963\u0981-\u0981\u09bc-\u09bc\u09c1-\u09c4\u09cd-\u09cd\u09e2-\u09e3\u0a01-\u0a02\u0a3c-\u0a3c\u0a41-\u0a42\u0a47-\u0a48\u0a4b-\u0a4d\u0a51-\u0a51\u0a70-\u0a71\u0a75-\u0a75\u0a81-\u0a82\u0abc-\u0abc\u0ac1-\u0ac5\u0ac7-\u0ac8\u0acd-\u0acd\u0ae2-\u0ae3\u0b01-\u0b01\u0b3c-\u0b3c\u0b3f-\u0b3f\u0b41-\u0b44\u0b4d-\u0b4d\u0b56-\u0b56\u0b62-\u0b63\u0b82-\u0b82\u0bc0-\u0bc0\u0bcd-\u0bcd\u0c00-\u0c00\u0c3e-\u0c40\u0c46-\u0c48\u0c4a-\u0c4d\u0c55-\u0c56\u0c62-\u0c63\u0c81-\u0c81\u0cbc-\u0cbc\u0cbf-\u0cbf\u0cc6-\u0cc6\u0ccc-\u0ccd\u0ce2-\u0ce3\u0d01-\u0d01\u0d41-\u0d44\u0d4d-\u0d4d\u0d62-\u0d63\u0dca-\u0dca\u0dd2-\u0dd4\u0dd6-\u0dd6\u0e31-\u0e31\u0e34-\u0e3a\u0e47-\u0e4e\u0eb1-\u0eb1\u0eb4-\u0eb9\u0ebb-\u0ebc\u0ec8-\u0ecd\u0f18-\u0f19\u0f35-\u0f35\u0f37-\u0f37\u0f39-\u0f39\u0f71-\u0f7e\u0f80-\u0f84\u0f86-\u0f87\u0f8d-\u0f97\u0f99-\u0fbc\u0fc6-\u0fc6\u102d-\u1030\u1032-\u1037\u1039-\u103a\u103d-\u103e\u1058-\u1059\u105e-\u1060\u1071-\u1074\u1082-\u1082\u1085-\u1086\u108d-\u108d\u109d-\u109d\u135d-\u135f\u1712-\u1714\u1732-\u1734\u1752-\u1753\u1772-\u1773\u17b4-\u17b5\u17b7-\u17bd\u17c6-\u17c6\u17c9-\u17d3\u17dd-\u17dd\u180b-\u180d\u18a9-\u18a9\u1920-\u1922\u1927-\u1928\u1932-\u1932\u1939-\u193b\u1a17-\u1a18\u1a1b-\u1a1b\u1a56-\u1a56\u1a58-\u1a5e\u1a60-\u1a60\u1a62-\u1a62\u1a65-\u1a6c\u1a73-\u1a7c\u1a7f-\u1a7f\u1ab0-\u1abd\u1b00-\u1b03\u1b34-\u1b34\u1b36-\u1b3a\u1b3c-\u1b3c\u1b42-\u1b42\u1b6b-\u1b73\u1b80-\u1b81\u1ba2-\u1ba5\u1ba8-\u1ba9\u1bab-\u1bad\u1be6-\u1be6\u1be8-\u1be9\u1bed-\u1bed\u1bef-\u1bf1\u1c2c-\u1c33\u1c36-\u1c37\u1cd0-\u1cd2\u1cd4-\u1ce0\u1ce2-\u1ce8\u1ced-\u1ced\u1cf4-\u1cf4\u1cf8-\u1cf9\u1dc0-\u1df5\u1dfc-\u1dff\u20d0-\u20dc\u20e1-\u20e1\u20e5-\u20f0\u2cef-\u2cf1\u2d7f-\u2d7f\u2de0-\u2dff\u302a-\u302d\u3099-\u309a\ua66f-\ua66f\ua674-\ua67d\ua69f-\ua69f\ua6f0-\ua6f1\ua802-\ua802\ua806-\ua806\ua80b-\ua80b\ua825-\ua826\ua8c4-\ua8c4\ua8e0-\ua8f1\ua926-\ua92d\ua947-\ua951\ua980-\ua982\ua9b3-\ua9b3\ua9b6-\ua9b9\ua9bc-\ua9bc\ua9e5-\ua9e5\uaa29-\uaa2e\uaa31-\uaa32\uaa35-\uaa36\uaa43-\uaa43\uaa4c-\uaa4c\uaa7c-\uaa7c\uaab0-\uaab0\uaab2-\uaab4\uaab7-\uaab8\uaabe-\uaabf\uaac1-\uaac1\uaaec-\uaaed\uaaf6-\uaaf6\uabe5-\uabe5\uabe8-\uabe8\uabed-\uabed\ufb1e-\ufb1e\ufe00-\ufe0f\ufe20-\ufe2d\U000101fd-\U000101fd\U000102e0-\U000102e0\U00010376-\U0001037a\U00010a01-\U00010a03\U00010a05-\U00010a06\U00010a0c-\U00010a0f\U00010a38-\U00010a3a\U00010a3f-\U00010a3f\U00010ae5-\U00010ae6\U00011001-\U00011001\U00011038-\U00011046\U0001107f-\U00011081\U000110b3-\U000110b6\U000110b9-\U000110ba\U00011100-\U00011102\U00011127-\U0001112b\U0001112d-\U00011134\U00011173-\U00011173\U00011180-\U00011181\U000111b6-\U000111be\U0001122f-\U00011231\U00011234-\U00011234\U00011236-\U00011237\U000112df-\U000112df\U000112e3-\U000112ea\U00011301-\U00011301\U0001133c-\U0001133c\U00011340-\U00011340\U00011366-\U0001136c\U00011370-\U00011374\U000114b3-\U000114b8\U000114ba-\U000114ba\U000114bf-\U000114c0\U000114c2-\U000114c3\U000115b2-\U000115b5\U000115bc-\U000115bd\U000115bf-\U000115c0\U00011633-\U0001163a\U0001163d-\U0001163d\U0001163f-\U00011640\U000116ab-\U000116ab\U000116ad-\U000116ad\U000116b0-\U000116b5\U000116b7-\U000116b7\U00016af0-\U00016af4\U00016b30-\U00016b36\U00016f8f-\U00016f92\U0001bc9d-\U0001bc9e\U0001d167-\U0001d169\U0001d17b-\U0001d182\U0001d185-\U0001d18b\U0001d1aa-\U0001d1ad\U0001d242-\U0001d244\U0001e8d0-\U0001e8d6\U000e0100-\U000e01ef];
Mc = [\u0903-\u0903\u093b-\u093b\u093e-\u0940\u0949-\u094c\u094e-\u094f\u0982-\u0983\u09be-\u09c0\u09c7-\u09c8\u09cb-\u09cc\u09d7-\u09d7\u0a03-\u0a03\u0a3e-\u0a40\u0a83-\u0a83\u0abe-\u0ac0\u0ac9-\u0ac9\u0acb-\u0acc\u0b02-\u0b03\u0b3e-\u0b3e\u0b40-\u0b40\u0b47-\u0b48\u0b4b-\u0b4c\u0b57-\u0b57\u0bbe-\u0bbf\u0bc1-\u0bc2\u0bc6-\u0bc8\u0bca-\u0bcc\u0bd7-\u0bd7\u0c01-\u0c03\u0c41-\u0c44\u0c82-\u0c83\u0cbe-\u0cbe\u0cc0-\u0cc4\u0cc7-\u0cc8\u0cca-\u0ccb\u0cd5-\u0cd6\u0d02-\u0d03\u0d3e-\u0d40\u0d46-\u0d48\u0d4a-\u0d4c\u0d57-\u0d57\u0d82-\u0d83\u0dcf-\u0dd1\u0dd8-\u0ddf\u0df2-\u0df3\u0f3e-\u0f3f\u0f7f-\u0f7f\u102b-\u102c\u1031-\u1031\u1038-\u1038\u103b-\u103c\u1056-\u1057\u1062-\u1064\u1067-\u106d\u1083-\u1084\u1087-\u108c\u108f-\u108f\u109a-\u109c\u17b6-\u17b6\u17be-\u17c5\u17c7-\u17c8\u1923-\u1926\u1929-\u192b\u1930-\u1931\u1933-\u1938\u19b0-\u19c0\u19c8-\u19c9\u1a19-\u1a1a\u1a55-\u1a55\u1a57-\u1a57\u1a61-\u1a61\u1a63-\u1a64\u1a6d-\u1a72\u1b04-\u1b04\u1b35-\u1b35\u1b3b-\u1b3b\u1b3d-\u1b41\u1b43-\u1b44\u1b82-\u1b82\u1ba1-\u1ba1\u1ba6-\u1ba7\u1baa-\u1baa\u1be7-\u1be7\u1bea-\u1bec\u1bee-\u1bee\u1bf2-\u1bf3\u1c24-\u1c2b\u1c34-\u1c35\u1ce1-\u1ce1\u1cf2-\u1cf3\u302e-\u302f\ua823-\ua824\ua827-\ua827\ua880-\ua881\ua8b4-\ua8c3\ua952-\ua953\ua983-\ua983\ua9b4-\ua9b5\ua9ba-\ua9bb\ua9bd-\ua9c0\uaa2f-\uaa30\uaa33-\uaa34\uaa4d-\uaa4d\uaa7b-\uaa7b\uaa7d-\uaa7d\uaaeb-\uaaeb\uaaee-\uaaef\uaaf5-\uaaf5\uabe3-\uabe4\uabe6-\uabe7\uabe9-\uabea\uabec-\uabec\U00011000-\U00011000\U00011002-\U00011002\U00011082-\U00011082\U000110b0-\U000110b2\U000110b7-\U000110b8\U0001112c-\U0001112c\U00011182-\U00011182\U000111b3-\U000111b5\U000111bf-\U000111c0\U0001122c-\U0001122e\U00011232-\U00011233\U00011235-\U00011235\U000112e0-\U000112e2\U00011302-\U00011303\U0001133e-\U0001133f\U00011341-\U00011344\U00011347-\U00011348\U0001134b-\U0001134d\U00011357-\U00011357\U00011362-\U00011363\U000114b0-\U000114b2\U000114b9-\U000114b9\U000114bb-\U000114be\U000114c1-\U000114c1\U000115af-\U000115b1\U000115b8-\U000115bb\U000115be-\U000115be\U00011630-\U00011632\U0001163b-\U0001163c\U0001163e-\U0001163e\U000116ac-\U000116ac\U000116ae-\U000116af\U000116b6-\U000116b6\U00016f51-\U00016f7e\U0001d165-\U0001d166\U0001d16d-\U0001d172];
Z = [\x20-\x20\xa0-\xa0\u1680-\u1680\u2000-\u200a\u2028-\u2029\u202f-\u202f\u205f-\u205f\u3000-\u3000];
Nd = [\x30-\x39\u0660-\u0669\u06f0-\u06f9\u07c0-\u07c9\u0966-\u096f\u09e6-\u09ef\u0a66-\u0a6f\u0ae6-\u0aef\u0b66-\u0b6f\u0be6-\u0bef\u0c66-\u0c6f\u0ce6-\u0cef\u0d66-\u0d6f\u0de6-\u0def\u0e50-\u0e59\u0ed0-\u0ed9\u0f20-\u0f29\u1040-\u1049\u1090-\u1099\u17e0-\u17e9\u1810-\u1819\u1946-\u194f\u19d0-\u19d9\u1a80-\u1a89\u1a90-\u1a99\u1b50-\u1b59\u1bb0-\u1bb9\u1c40-\u1c49\u1c50-\u1c59\ua620-\ua629\ua8d0-\ua8d9\ua900-\ua909\ua9d0-\ua9d9\ua9f0-\ua9f9\uaa50-\uaa59\uabf0-\uabf9\uff10-\uff19\U000104a0-\U000104a9\U00011066-\U0001106f\U000110f0-\U000110f9\U00011136-\U0001113f\U000111d0-\U000111d9\U000112f0-\U000112f9\U000114d0-\U000114d9\U00011650-\U00011659\U000116c0-\U000116c9\U000118e0-\U000118e9\U00016a60-\U00016a69\U00016b50-\U00016b59\U0001d7ce-\U0001d7ff];
Nl = [\u16ee-\u16f0\u2160-\u2182\u2185-\u2188\u3007-\u3007\u3021-\u3029\u3038-\u303a\ua6e6-\ua6ef\U00010140-\U00010174\U00010341-\U00010341\U0001034a-\U0001034a\U000103d1-\U000103d5\U00012400-\U0001246e];
Pc = [\x5f-\x5f\u203f-\u2040\u2054-\u2054\ufe33-\ufe34\ufe4d-\ufe4f\uff3f-\uff3f];
*/

struct jinput_t {
  unsigned char buf[SIZE + YYMAXFILL];
  const unsigned char *lim;
  const unsigned char *cur;
  const unsigned char *mar;
  const unsigned char *tok;
  const unsigned char *sol;
  /*!stags:re2c format = "const unsigned char *@@;"; */
  long offset;
  int  row;
  bool eof;

  const char *filename;
  FILE *const file;

  jinput_t(const char *fn, FILE *f, int start = SIZE, int end = SIZE)
   : buf(), lim(buf + end), cur(buf + start), mar(buf + start), tok(buf + start), sol(buf + start),
     /*!stags:re2c format = "@@(NULL)"; separator = ","; */,
     offset(-start), row(1), eof(false), filename(fn), file(f) { }
  jinput_t(const char *fn, const unsigned char *buf_, int end)
   : buf(), lim(buf_ + end), cur(buf_), mar(buf_), tok(buf_), sol(buf_),
     /*!stags:re2c format = "@@(NULL)"; separator = ","; */,
     offset(0), row(1), eof(true), filename(fn), file(0) { }

  bool __attribute__ ((noinline)) fill(size_t need);

  Coordinates coord() const { return Coordinates(row, 1 + cur - sol, offset + cur - &buf[0]); }
};

#define SYM_LOCATION Location(in.filename, start, in.coord()-1)
#define mkSym2(x, v) JSymbol(x, SYM_LOCATION, std::move(v))
#define mkSym(x) JSymbol(x, SYM_LOCATION)

bool jinput_t::fill(size_t need) {
  if (eof) {
    return false;
  }
  const size_t free = tok - buf;
  if (free < need) {
    return false;
  }
  memmove(buf, tok, lim - tok);
  lim -= free;
  cur -= free;
  mar -= free;
  tok -= free;
  sol -= free;
  /*!stags:re2c format = "if (@@) @@ -= free;"; */
  offset += free;
  lim += fread(buf + (lim - buf), 1, free, file);
  if (lim < buf + SIZE) {
    eof = true;
    memset(buf + (lim - buf), 0, YYMAXFILL);
    lim += YYMAXFILL;
  }
  return true;
}

static bool lex_jstr(JLexer &lex, std::string &out, unsigned char eos)
{
  jinput_t &in = *lex.engine.get();
  std::string slice;

  while (true) {
    in.tok = in.cur;
    /*!re2c
        re2c:define:YYCURSOR = in.cur;
        re2c:define:YYMARKER = in.mar;
        re2c:define:YYLIMIT = in.lim;
        re2c:yyfill:enable = 1;
        re2c:define:YYFILL = "if (!in.fill(@@)) return false;";
        re2c:define:YYFILL:naked = 1;

        // LineTerminator ::
        //   <LF>
        //   <CR>
        //   <LS>
        //   <PS>
        LineTerminator = [\n\r\u2028\u2029];

        // LineTerminatorSequence ::
        //   <LF>
        //   <CR> [lookahead ∉ <LF> ]
        //   <LS>
        //   <PS>
        //   <CR> <LF>
        LineTerminatorSequence = [\n\r\u2028\u2029] | "\r\n";

        // LineContinuation ::
        //   \ LineTerminatorSequence
        LineContinuation = "\\" LineTerminatorSequence;

        // HexDigit ::
        //   one of 0 1 2 3 4 5 6 7 8 9 a b c d e f A B C D E F
        HexDigit = [0-9a-fA-F];

        // SingleEscapeCharacter ::
        //   one of ' " \ b f n r t v
        // EscapeCharacter ::
        //   SingleEscapeCharacter
        //   DecimalDigit
        //   x
        //   u
        // NonEscapeCharacter ::
        //   SourceCharacter but not one of EscapeCharacter or LineTerminator
        NonEscapeCharacter = [^'"\\bfnrtv0-9xu\n\r\u2028\u2029\x00];

        // CharacterEscapeSequence ::
        //   SingleEscapeCharacter
        //   NonEscapeCharacter

        // HexEscapeSequence ::
        //   x HexDigit HexDigit
        HexEscapeSequence = "x" HexDigit{2};

        // UnicodeEscapeSequence ::
        //   u HexDigit HexDigit HexDigit HexDigit
        UnicodeEscapeSequence = "u" HexDigit{4};

        // EscapeSequence ::
        //   CharacterEscapeSequence
        //   0 [lookahead ∉ DecimalDigit]
        //   HexEscapeSequence
        //   UnicodeEscapeSequence

        // JSON5DoubleStringCharacter::
        //   SourceCharacter but not one of " or \ or LineTerminator
        //   \ EscapeSequence
        //   LineContinuation
        //   U+2028
        //   U+2029
        // JSON5SingleStringCharacter::
        //   SourceCharacter but not one of ' or \ or LineTerminator
        //   \ EscapeSequence
        //   LineContinuation
        //   U+2028
        //   U+2029

        // JSON5DoubleStringCharacters::
        //   JSON5DoubleStringCharacter JSON5DoubleStringCharactersopt
        // JSON5SingleStringCharacters::
        //   JSON5SingleStringCharacter JSON5SingleStringCharactersopt

        *                { return false; }
        LineContinuation { continue; }
        ["'] {
          if (in.cur[-1] == eos) {
             break;
          } else {
             slice.push_back(in.cur[-1]);
             continue;
          }
        }

	// Two surrogates => one character
        "\\u" [dD] [89abAB] HexDigit{2} "\\u" [dD] [c-fC-F] HexDigit{2} {
          uint32_t lo = lex_hex(in.tok,   in.tok+6);
          uint32_t hi = lex_hex(in.tok+6, in.tok+12);
          uint32_t x = ((lo & 0x3ff) << 10) + (hi & 0x3ff) + 0x10000;
          if (!push_utf8(slice, x)) return false;
          continue;
        }
	"\\u" HexDigit{4}       { if (!push_utf8(slice, lex_hex(in.tok, in.cur))) return false; continue; }
	"\\x" HexDigit{2}       { if (!push_utf8(slice, lex_hex(in.tok, in.cur))) return false; continue; }
        "\\0" / [^0-9]          { slice.push_back('\0'); continue; }
        "\\b"                   { slice.push_back('\b'); continue; }
        "\\t"                   { slice.push_back('\t'); continue; }
        "\\n"                   { slice.push_back('\n'); continue; }
        "\\v"                   { slice.push_back('\v'); continue; }
        "\\f"                   { slice.push_back('\f'); continue; }
        "\\r"                   { slice.push_back('\r'); continue; }
        "\\'"                   { slice.push_back('\''); continue; }
        "\\\""                  { slice.push_back('"');  continue; }
        "\\\\"                  { slice.push_back('\\'); continue; }
        "\\" NonEscapeCharacter { slice.append(in.tok+1, in.cur);  continue; }
	"\\"                    { return false; }
        [^\x00\r\n]             { slice.append(in.tok, in.cur); continue; }
    */
  }

  out = std::move(slice);
  return true;
}

static void lex_jcomment(jinput_t &in) {
  unsigned const char *s = in.tok;
  unsigned const char *e = in.cur;
  while (s != e) {
    /*!re2c
        re2c:yyfill:enable = 0;
        re2c:define:YYCURSOR = s;
        *               { continue; }
        LineTerminator  { ++in.row; in.sol = s; continue; }
    */
  }
}

static JSymbol lex_json(JLexer &lex) {
  jinput_t &in = *lex.engine.get();
  Coordinates start;
top:
  start = in.coord();
  in.tok = in.cur;

  /*!re2c
      re2c:define:YYCURSOR = in.cur;
      re2c:define:YYMARKER = in.mar;
      re2c:define:YYLIMIT = in.lim;
      re2c:yyfill:enable = 1;
      re2c:define:YYFILL = "if (!in.fill(@@)) return mkSym(JSON_ERROR);";
      re2c:define:YYFILL:naked = 1;

      // WhiteSpace ::
      //   <TAB>
      //   <VT>
      //   <FF>
      //   <SP>
      //   <NBSP>
      //   <BOM>
      //   <USP>
      WhiteSpace = [\t\v\f \u00a0\ufeff] | Z;

      // MultiLineNotForwardSlashOrAsteriskChar ::
      //   SourceCharacter but not one of / or *
      // PostAsteriskCommentChars ::
      //   MultiLineNotForwardSlashOrAsteriskChar MultiLineCommentCharsopt
      //   * PostAsteriskCommentCharsopt
      // => PostAsteriskCommentCharsopt = "*"* ([^/*] MultiLineCommentCharsopt)?
      // MultiLineNotAsteriskChar ::
      //   SourceCharacter but not *
      // MultiLineCommentChars ::
      //   MultiLineNotAsteriskChar MultiLineCommentCharsopt
      //   * PostAsteriskCommentCharsopt
      // => MultiLineCommentCharsopt = [^*]* ("*" PostAsteriskCommentCharsopt)?

      // To cut the cyclic definition above, note:
      //   a = c* (x b)?
      //   b = d* (y a)?
      //   => a = c* (x d* (y c* (x d* (y c* (x d* ...)?)?)?)?)?
      //   => a = c* (x d* y c*)* (x d*)?
      //     where c = [^*], x = "*", d = "*", y = [^/*]
      //   => MultiLineCommentCharsopt = [^*]* ("*" "*"* [^/*] [^*]*)* ("*" "*"*)?
      MultiLineCommentCharsopt = [^\x00*]* ("*"+ [^\x00/*] [^\x00*]*)* "*"*;

      // MultiLineComment ::
      //   /* MultiLineCommentCharsopt */
      MultiLineComment = "/*" MultiLineCommentCharsopt "*/";

      // SingleLineCommentChar ::
      //   SourceCharacter but not LineTerminator
      // SingleLineCommentChars ::
      //   SingleLineCommentChar SingleLineCommentCharsopt
      // SingleLineComment ::
      //   // SingleLineCommentCharsopt
      SingleLineComment = "//" [^\n\r\u2028\u2029\x00]*;

      // Comment ::
      //   MultiLineComment
      //   SingleLineComment
      Comment = MultiLineComment | SingleLineComment;

      // HexIntegerLiteral ::
      //   0x HexDigit
      //   0X HexDigit
      //   HexIntegerLiteral HexDigit
      HexIntegerLiteral = "0" [xX] HexDigit+;

      // DecimalDigit ::
      //   one of 0 1 2 3 4 5 6 7 8 9
      DecimalDigit = [0-9];

      // NonZeroDigit ::
      //   one of 1 2 3 4 5 6 7 8 9
      NonZeroDigit = [1-9];

      // DecimalDigits ::
      //   DecimalDigit
      //   DecimalDigits DecimalDigit
      DecimalDigits = DecimalDigit+;

      // DecimalIntegerLiteral ::
      //   0
      //   NonZeroDigit DecimalDigitsopt
      DecimalIntegerLiteral = "0" | NonZeroDigit DecimalDigits?;

      // SignedInteger ::
      //   DecimalDigits
      //   + DecimalDigits
      //   - DecimalDigits
      // ExponentIndicator ::
      //   one of e E
      // ExponentPart ::
      //   ExponentIndicator SignedInteger
      ExponentPart = [eE] [+-]? DecimalDigits;

      // DecimalLiteral ::
      //   DecimalIntegerLiteral . DecimalDigitsopt ExponentPartopt
      //   . DecimalDigits ExponentPartopt
      //   DecimalIntegerLiteral ExponentPartopt
      DecimalLiteral = (DecimalIntegerLiteral "." DecimalDigits? ExponentPart?)
                     | ("." DecimalDigits ExponentPart?)
                     | DecimalIntegerLiteral ExponentPart?;

      // NumericLiteral ::
      //   DecimalLiteral
      //   HexIntegerLiteral
      NumericLiteral = DecimalLiteral | HexIntegerLiteral;

      // JSON5NumericLiteral::
      //   NumericLiteral
      //   Infinity
      //   NaN
      JSON5NumericLiteral = NumericLiteral | "Infinity" | "NaN";

      // JSON5Number::
      //   JSON5NumericLiteral
      //   + JSON5NumericLiteral
      //   - JSON5NumericLiteral
      JSON5Number = [+-]? JSON5NumericLiteral;

      // UnicodeLetter ::
      //   any character in the Unicode categories “Uppercase letter (Lu)”, “Lowercase letter (Ll)”, “Titlecase letter (Lt)”, “Modifier letter (Lm)”, “Other letter (Lo)”, or “Letter number (Nl)”.
      UnicodeLetter = Lu | Ll | Lt | Lm | Lo | Nl;
      // UnicodeCombiningMark ::
      //   any character in the Unicode categories “Non-spacing mark (Mn)” or “Combining spacing mark (Mc)”
      UnicodeCombiningMark = Mn | Mc;
      // UnicodeDigit ::
      //   any character in the Unicode category “Decimal number (Nd)”
      UnicodeDigit = Nd;
      // UnicodeConnectorPunctuation ::
      //   any character in the Unicode category “Connector punctuation (Pc)”
      UnicodeConnectorPunctuation = Pc;

      // IdentifierStart ::
      //   UnicodeLetter
      //   $
      //   _
      //   \ UnicodeEscapeSequence
      IdentifierStart = UnicodeLetter | [$_] | ("\\" UnicodeEscapeSequence);

      // IdentifierPart ::
      //   IdentifierStart
      //   UnicodeCombiningMark
      //   UnicodeDigit
      //   UnicodeConnectorPunctuation
      //   <ZWNJ>
      //   <ZWJ>
      IdentifierPart = IdentifierStart | UnicodeCombiningMark | UnicodeDigit | UnicodeConnectorPunctuation | [\u200C\u200D];

      // IdentifierName ::
      //   IdentifierStart
      //   IdentifierName IdentifierPart
      // JSON5Identifier::
      //   IdentifierName
      JSON5Identifier = IdentifierStart IdentifierPart*;

      // JSON5SourceCharacter::
      //   SourceCharacter
      // SourceCharacter ::
      //   any Unicode code unit
      * { return mkSym(JSON_ERROR); }

      // Minor extension to json5; interpret NULL as EOF
      "\x00" { return mkSym((in.lim - in.tok == YYMAXFILL) ? JSON_END : JSON_ERROR); }

      // JSON5InputElement::
      //   WhiteSpace
      //   LineTerminator
      //   Comment
      //   JSON5Token
      WhiteSpace      { goto top; }
      LineTerminator  { ++in.row; in.sol = in.cur; goto top; }
      Comment         {
        lex_jcomment(in);
        goto top;
      }

      // JSON5Token::
      //   JSON5Identifier
      //   JSON5Punctuator
      //   JSON5String
      //   JSON5Number

      // Special case the integers
      [+-]? (DecimalIntegerLiteral|HexIntegerLiteral) {
        std::string value(in.tok[0] == '+' ? in.tok+1 : in.tok, in.cur);
        return mkSym2(JSON_INTEGER, value);
      }
      [+-]? NumericLiteral {
        std::string value(in.tok[0] == '+' ? in.tok+1 : in.tok, in.cur);
        return mkSym2(JSON_DOUBLE, value);
      }
      [+-]? "Infinity" {
        std::string value(in.tok[0]=='-'?"-":"+");
        return mkSym2(JSON_INFINITY, value);
      }
      [+-]? "NaN" {
        return mkSym(JSON_NAN);
      }

      // JSON5Punctuator::
      //   one of {}[]:,
      "{"             { return mkSym(JSON_BOPEN);   }
      "}"             { return mkSym(JSON_BCLOSE);  }
      "["             { return mkSym(JSON_SOPEN);   }
      "]"             { return mkSym(JSON_SCLOSE);  }
      ":"             { return mkSym(JSON_COLON);   }
      ","             { return mkSym(JSON_COMMA);   }

      // JSON5Null::
      //   NullLiteral
      // NullLiteral ::
      //   null
      "null"          { return mkSym(JSON_NULLVAL); }

      // JSON5Boolean::
      //   BooleanLiteral
      // BooleanLiteral ::
      //   true
      //   false
      "false"         { return mkSym(JSON_FALSE);   }
      "true"          { return mkSym(JSON_TRUE);    }

      JSON5Identifier {
        std::string value(in.tok, in.cur);
        return mkSym2(JSON_ID, value);
      }

      // JSON5String::
      //  " JSON5DoubleStringCharactersopt "
      //  ' JSON5SingleStringCharactersopt '

      ['"] {
        std::string out;
        bool ok = lex_jstr(lex, out, in.cur[-1]);
        return mkSym2(ok ? JSON_STR : JSON_ERROR, out);
      }
  */
}

JLexer::JLexer(const char *file)
 : engine(new jinput_t(file, fopen(file, "r"))), next(JSON_ERROR, Location(file, Coordinates(), Coordinates())), fail(false)
{
  if (engine->file) consume();
}

JLexer::JLexer(const std::string &body)
  : engine(new jinput_t("string", reinterpret_cast<const unsigned char *>(body.c_str()), YYMAXFILL+body.size())), next(JSON_ERROR, LOCATION), fail(false)
{
  consume();
}

JLexer::~JLexer() {
  if (engine->file) fclose(engine->file);
}

void JLexer::consume() {
  next = lex_json(*this);
}
