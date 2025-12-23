<?php
header('Content-Type: application/json');
header('Access-Control-Allow-Origin: *');
header('Access-Control-Allow-Methods: POST, OPTIONS');
header('Access-Control-Allow-Headers: content-type');

if ($_SERVER['REQUEST_METHOD'] === 'OPTIONS') {
  http_response_code(204);
  exit;
}

$baseDir = dirname(__FILE__) . '/../data';
if (!is_dir($baseDir)) {
  @mkdir($baseDir, 0775, true);
}
$telemetryFile = $baseDir . '/telemetry.json';

// Get raw POST data
$raw = file_get_contents('php://input');
$input = json_decode($raw, true);

if (!is_array($input)) {
  // Fallback to $_POST if not raw JSON
  $input = $_POST;
}

// Prepare data to save
$data = [
  'updated_at' => date('c'),
  'ip_address' => $_SERVER['REMOTE_ADDR']
];

// Merge known fields
$allowedKeys = ['device_id', 'temp', 'humidity', 'setpoint', 'heat_status', 'cool_status', 'fan_status'];
foreach ($allowedKeys as $key) {
  if (isset($input[$key])) {
    $data[$key] = $input[$key];
  }
}

// Preserve existing data if partial update (optional, but safer to just overwrite for telemetry)
// But let's check if we want to merge or overwrite. Telemetry usually overwrites the "current state".
// But we might want to keep some static fields if they aren't sent every time.
$currentData = [];
if (file_exists($telemetryFile)) {
  $currentData = json_decode(@file_get_contents($telemetryFile), true);
  if (!is_array($currentData))
    $currentData = [];
}

$newData = array_merge($currentData, $data);

if (file_put_contents($telemetryFile, json_encode($newData, JSON_UNESCAPED_SLASHES | JSON_PRETTY_PRINT))) {
  echo json_encode(['ok' => true, 'message' => 'Telemetry saved']);
} else {
  http_response_code(500);
  echo json_encode(['ok' => false, 'message' => 'Failed to save file']);
}
