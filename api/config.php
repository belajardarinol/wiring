<?php
header('Content-Type: application/json');
header('Access-Control-Allow-Origin: *');
header('Access-Control-Allow-Methods: GET, POST, OPTIONS');
header('Access-Control-Allow-Headers: content-type, authorization');

if ($_SERVER['REQUEST_METHOD'] === 'OPTIONS') { http_response_code(204); exit; }

$baseDir = dirname(__FILE__) . '/../data';
if (!is_dir($baseDir)) { @mkdir($baseDir, 0775, true); }
$configFile = $baseDir . '/config.json';

$default = [
  'setpoint' => 30.0,
  'heatTemp' => 28.0,
  'coolTemp' => 32.0,
  'lowerLimit' => 25.0,
  'upperLimit' => 35.0,
  'displayBrightness' => 15
];

if ($_SERVER['REQUEST_METHOD'] === 'GET') {
  $cfg = $default;
  if (file_exists($configFile)) {
    $data = json_decode(@file_get_contents($configFile), true);
    if (is_array($data)) { $cfg = array_merge($cfg, $data); }
  }
  echo json_encode(['ok'=>true,'config'=>$cfg]);
  exit;
}

$raw = file_get_contents('php://input');
$input = json_decode($raw, true);
if (!is_array($input)) { $input = $_POST; }

$cfg = $default;
if (file_exists($configFile)) {
  $data = json_decode(@file_get_contents($configFile), true);
  if (is_array($data)) { $cfg = array_merge($cfg, $data); }
}

$allowedKeys = array_keys($default);
$changed = [];
foreach ($allowedKeys as $key) {
  if (isset($input[$key])) {
    $val = $input[$key];
    if (is_numeric($default[$key])) { $val = $val + 0; }
    $cfg[$key] = $val;
    $changed[$key] = $val;
  }
}
@file_put_contents($configFile, json_encode($cfg, JSON_UNESCAPED_SLASHES|JSON_UNESCAPED_UNICODE|JSON_PRETTY_PRINT));
echo json_encode(['ok'=>true, 'changed'=>$changed, 'config'=>$cfg]);
