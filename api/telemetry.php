<?php
header('Content-Type: application/json');
header('Access-Control-Allow-Origin: *');
header('Access-Control-Allow-Methods: POST, OPTIONS');
header('Access-Control-Allow-Headers: content-type, authorization');

if ($_SERVER['REQUEST_METHOD'] === 'OPTIONS') { http_response_code(204); exit; }

$raw = file_get_contents('php://input');
$input = json_decode($raw, true);
if (!is_array($input)) { $input = $_POST; }

$temp = isset($input['temp']) ? floatval($input['temp']) : null;
$humidity = isset($input['humidity']) ? floatval($input['humidity']) : null;
$setpoint = isset($input['setpoint']) ? floatval($input['setpoint']) : null;
$device_id = isset($input['device_id']) ? preg_replace('/[^a-zA-Z0-9_\-\.]/', '', $input['device_id']) : 'device-1';
if ($temp === null || $humidity === null) {
  http_response_code(400);
  echo json_encode(['ok'=>false,'error'=>'Missing temp or humidity']);
  exit;
}

$baseDir = dirname(__FILE__) . '/../data';
if (!is_dir($baseDir)) { @mkdir($baseDir, 0775, true); }
$telemetryFile = $baseDir . '/telemetry.json';
$logFile = $baseDir . '/telemetry-log.jsonl';

$payload = [
  'device_id' => $device_id,
  'temp' => $temp,
  'humidity' => $humidity,
  'setpoint' => $setpoint,
  'updated_at' => date('c')
];

@file_put_contents($telemetryFile, json_encode($payload, JSON_UNESCAPED_SLASHES|JSON_UNESCAPED_UNICODE));
@file_put_contents($logFile, json_encode($payload, JSON_UNESCAPED_SLASHES|JSON_UNESCAPED_UNICODE) . PHP_EOL, FILE_APPEND);

echo json_encode(['ok'=>true]);
