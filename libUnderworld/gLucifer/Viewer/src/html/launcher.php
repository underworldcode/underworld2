<?php
  $port = isset($_GET['port']) ? (int)$_GET['port'] : 8081;
  $width = isset($_GET['width']) ? (int)$_GET['width'] : 1024;
  $height = isset($_GET['height']) ? (int)$_GET['height'] : 768;
  $db = isset($_GET['db']) ? $_GET['db'] : "";
  $gldir = isset($_GET['gldir']) ? $_GET['gldir'] : "";

  $cmd = "$gldir/gLucifer -v -p$port -n10 -r$width,$height $db";
  //$cmd = "$dir/gLuciferOS -v -h -p$port";
  //$cmd = "/opt/VirtualGL/bin/vglrun -display :0 $dir/gLucifer -v -h -p$port";

  echo "Running $cmd on $port...";
  //exec($cmd . " > /dev/null &", $output, $ret);
  $ret = exec_bg($cmd, "/tmp/$port.log");
  //echo "exists: " . exists($ret);
  //echo "$dir/$db";
  header('Location: ' . $_SERVER['HTTP_REFERER']);
  die();

function exec_bg($cmd, $output)
{
  // Execute $cmd in bg, returns PID
  return (int) exec("$cmd > $output 2>&1 & echo $!");
}

function exists($pid)
{
  // Checks process with ID $pid
  return (bool)file_exists("/proc/$pid");
}

?>
