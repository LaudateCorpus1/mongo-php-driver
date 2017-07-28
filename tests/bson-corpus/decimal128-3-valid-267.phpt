--TEST--
Decimal128: [basx068] examples
--DESCRIPTION--
Generated by scripts/convert-bson-corpus-tests.php

DO NOT EDIT THIS FILE
--FILE--
<?php

require_once __DIR__ . '/../utils/tools.php';

$canonicalBson = hex2bin('180000001364003200000000000000000000000000323000');
$canonicalExtJson = '{"d" : {"$numberDecimal" : "0.0000050"}}';
$degenerateExtJson = '{"d" : {"$numberDecimal" : "50E-7"}}';

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
180000001364003200000000000000000000000000323000
{"d":{"$numberDecimal":"0.0000050"}}
180000001364003200000000000000000000000000323000
180000001364003200000000000000000000000000323000
===DONE===