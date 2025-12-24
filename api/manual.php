<?php
header('Content-Type: application/json');
header('Access-Control-Allow-Origin: *');
header('Access-Control-Allow-Methods: GET, POST, OPTIONS');
header('Access-Control-Allow-Headers: content-type, authorization');

if ($_SERVER['REQUEST_METHOD'] === 'OPTIONS') {
  http_response_code(204);
  exit;
}

$baseDir = dirname(__FILE__) . '/../data';
if (!is_dir($baseDir)) {
  @mkdir($baseDir, 0775, true);
}
$manualFile = $baseDir . '/manual.json';

// Default manual state (all off, auto mode)
$default = [
  'manual' => false,
  'fan1' => 0,
  'fan2' => 0,
  'fan3' => 0,
  'fan4' => 0,
  'fan5' => 0,
  'fan6' => 0,
  'heater7' => 0,
  'cooling' => 0,
  'updated_at' => date('c')
];

// Load existing state if any
$state = $default;
if (file_exists($manualFile)) {
  $data = json_decode(@file_get_contents($manualFile), true);
  if (is_array($data)) {
    $state = array_merge($state, $data);
  }
}

if ($_SERVER['REQUEST_METHOD'] === 'GET') {
  // Return flat JSON for Arduino to parse easily
  echo json_encode($state);
  exit;
}

// POST: update manual state
$raw = file_get_contents('php://input');
$input = json_decode($raw, true);
if (!is_array($input)) {
  $input = $_POST;
}

$boolKeys = ['manual', 'fan1', 'fan2', 'fan3', 'fan4', 'fan5', 'fan6', 'heater7', 'cooling'];
foreach ($boolKeys as $key) {
  if (isset($input[$key])) {
    $val = $input[$key];
    $state[$key] = ($val === 1 || $val === '1' || $val === true || $val === 'true') ? 1 : 0;
  }
}
$state['updated_at'] = date('c');

@file_put_contents($manualFile, json_encode($state, JSON_UNESCAPED_SLASHES | JSON_UNESCAPED_UNICODE | JSON_PRETTY_PRINT));

echo json_encode(['ok' => true, 'state' => $state]);
