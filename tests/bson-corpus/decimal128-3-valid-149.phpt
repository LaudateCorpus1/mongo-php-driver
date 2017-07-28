--TEST--
Decimal128: [basx262] Numbers with E
--DESCRIPTION--
Generated by scripts/convert-bson-corpus-tests.php

DO NOT EDIT THIS FILE
--FILE--
<?php

require_once __DIR__ . '/../utils/tools.php';

$canonicalBson = hex2bin('18000000136400F104000000000000000000000000483000');
$canonicalExtJson = '{"d" : {"$numberDecimal" : "1.265E+7"}}';
$degenerateExtJson = '{"d" : {"$numberDecimal" : "0.1265E+8"}}';

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
18000000136400f104000000000000000000000000483000
{"d":{"$numberDecimal":"1.265E+7"}}
18000000136400f104000000000000000000000000483000
18000000136400f104000000000000000000000000483000
===DONE===