--TEST--
Decimal128: [basx678] Zeros
--DESCRIPTION--
Generated by scripts/convert-bson-corpus-tests.php

DO NOT EDIT THIS FILE
--FILE--
<?php

require_once __DIR__ . '/../utils/tools.php';

$canonicalBson = hex2bin('1800000013640000000000000000000000000000002C3000');
$canonicalExtJson = '{"d" : {"$numberDecimal" : "0E-10"}}';
$degenerateExtJson = '{"d" : {"$numberDecimal" : "0.00E-8"}}';

// Canonical BSON -> Native -> Canonical BSON 
echo bin2hex(fromPHP(toPHP($canonicalBson))), "\n";

// Canonical BSON -> Canonical extJSON 
echo json_canonicalize(toCanonicalJSON($canonicalBson)), "\n";

// Canonical extJSON -> Canonical BSON 
echo bin2hex(fromJSON($canonicalExtJson)), "\n";

// Degenerate extJSON -> Canonical BSON 
echo bin2hex(fromJSON($degenerateExtJson)), "\n";

?>
===DONE===
<?php exit(0); ?>
--EXPECT--
1800000013640000000000000000000000000000002c3000
{"d":{"$numberDecimal":"0E-10"}}
1800000013640000000000000000000000000000002c3000
1800000013640000000000000000000000000000002c3000
===DONE===