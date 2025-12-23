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
$configFile = $baseDir . '/config.json';

$default = [
  // Function 02 - Required Temperature
  'setpoint' => 30.0,
  // Function 03 - Heat
  'heatTemp' => 28.0,
  // Function 04-08 - Fan temperatures
  'fan1Temp' => 31.0,
  'fan2Temp' => 32.0,
  'fan3Temp' => 33.0,
  'fan4Temp' => 34.0,
  'fan5Temp' => 35.0,
  // Function 09-10 - Fan timing
  'fanOnTime' => 60,
  'fanOffTime' => 120,
  // Function 11 - Humidity
  'humiditySet' => 65,
  // Function 12-14 - Cooling
  'coolTemp' => 32.0,
  'coolOnTime' => 30,
  'coolOffTime' => 60,
  // Function 15-16 - Alarms
  'lowAlarm' => 15.0,
  'highAlarm' => 40.0,
  // Function 17 - Water
  'waterClock' => 0,
  // Function 18-20 - Feed
  'feedMult' => 1,
  'dailyFeed' => 0,
  'totalFeed' => 0,
  // Function 21 - Day 1 Temperature
  'day1Temp' => 33.0,
  // Function 31-32
  'growDay' => 1,
  'resetTime' => 0,
  // Hidden functions 33-40
  'lockCode' => 0,
  'tempOffset' => 0,
  'sensorType' => 0,
  'alarmDelay' => 60,
  'feedMode' => 0,
  'fanMode' => 0,
  'alarmType' => 0,
  'displayBrightness' => 15,
  // Legacy
  'lowerLimit' => 25.0,
  'upperLimit' => 35.0
];

if ($_SERVER['REQUEST_METHOD'] === 'GET') {
  $cfg = $default;
  if (file_exists($configFile)) {
    $data = json_decode(@file_get_contents($configFile), true);
    if (is_array($data)) {
      $cfg = array_merge($cfg, $data);
    }
  }
  echo json_encode(['ok' => true, 'config' => $cfg]);
  exit;
}

$raw = file_get_contents('php://input');
$input = json_decode($raw, true);
if (!is_array($input)) {
  $input = $_POST;
}

$cfg = $default;
if (file_exists($configFile)) {
  $data = json_decode(@file_get_contents($configFile), true);
  if (is_array($data)) {
    $cfg = array_merge($cfg, $data);
  }
}

$allowedKeys = array_keys($default);
$changed = [];
foreach ($allowedKeys as $key) {
  if (isset($input[$key])) {
    $val = $input[$key];
    if (is_numeric($default[$key])) {
      $val = $val + 0;
    }
    $cfg[$key] = $val;
    $changed[$key] = $val;
  }
}
@file_put_contents($configFile, json_encode($cfg, JSON_UNESCAPED_SLASHES | JSON_UNESCAPED_UNICODE | JSON_PRETTY_PRINT));
echo json_encode(['ok' => true, 'changed' => $changed, 'config' => $cfg]);
