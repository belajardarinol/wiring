<?php
header('Content-Type: application/json');
header('Access-Control-Allow-Origin: *');

$baseDir = dirname(__FILE__) . '/../data';
$telemetryFile = $baseDir . '/telemetry.json';
$configFile = $baseDir . '/config.json';

$telemetry = [
  'device_id' => null,
  'temp' => null,
  'humidity' => null,
  'setpoint' => null,
  'updated_at' => null,
];
if (file_exists($telemetryFile)) {
  $data = json_decode(@file_get_contents($telemetryFile), true);
  if (is_array($data)) { $telemetry = array_merge($telemetry, $data); }
}

$config = [
  'setpoint' => 30.0,
  'heatTemp' => 28.0,
  'coolTemp' => 32.0,
  'lowerLimit' => 20.0,
  'upperLimit' => 39.0,
  'displayBrightness' => 15
];
if (file_exists($configFile)) {
  $cfgData = json_decode(@file_get_contents($configFile), true);
  if (is_array($cfgData)) { $config = array_merge($config, $cfgData); }
}

echo json_encode([
  'ok' => true,
  'telemetry' => $telemetry,
  'config' => $config,
  'server_time' => date('c'),
]);
