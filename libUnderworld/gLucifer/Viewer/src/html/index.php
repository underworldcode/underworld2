<!DOCTYPE html>
<html>
<head>
<title>gLucifer Launcher</title>
<style>
body {
  margin: 0 auto;
  padding: 5px;
  font-family: "Helvetica Neue", Helvetica, Arial, sans-serif;
}

hr {border-top: 1px solid #eee}
</style>

</head>

<body>
<div style="float: right">
<input type="button" onclick="launch();" value="Run gLucifer">
</div>

<b>Running Instances</b>
<hr>

<div id="instances">
<?php
  $gldir = isset($_GET['gldir']) ? $_GET['gldir'] : "";
  $db = isset($_GET['db']) ? $_GET['db'] : "";
  $kill = isset($_GET['kill']) ? (int)$_GET['kill'] : -1;

  $ret = exec("ps -ax | grep 'gLucifer.*-p[0-9]\{4\}.*' ", $output);
  $ports = array();

  foreach ($output as $row)
  {
    $parts = preg_split('/\s-p/', $row);
    $pid = intval($parts[0]);
    $port = intval($parts[1]);
    $ports[$port] = true;
    preg_match('/gLucifer.*/', $row, $matches);
    $cmd = $matches[0];
    if ($pid == $kill)
    {
      echo "<del>";
      posix_kill($pid, 15);
      //posix_kill($pid, 9);
    }
    //echo "<br>PROC $pid PORT $port CMD $cmd";
    $url = "http://{$_SERVER['HTTP_HOST']}:$port";
    echo "<a href='#' onclick='urlOpen(\"$url\");'>[view]</a>";
    //echo " <a href='http://{$_SERVER['HTTP_HOST']}:$port/command=quit&0'>kill</a>";
    echo " <a href='index.php?kill=$pid&db=$db&gldir=$gldir'>[kill]</a>";
    echo " $cmd <br>";
    if ($pid == $kill) echo "</del>";
  }

  //Next next free port
  $nextport = 8080;
  while ($ports[$nextport]) $nextport++;
?>

</div>

<script type="text/javascript">
var port = <?= $nextport ?>;
var gldir = "<?= $gldir ?>";
var defaultdb = "<?= $db ?>";

function launch(db) {
  var width = window.innerWidth;
  var height = window.innerHeight;
  if (parent.document) {
    //Running in iframe, use parent dims
    height = parent.document.body.clientHeight;
    width = parent.document.body.clientWidth;
  }

  if (!db) db = defaultdb;

  location.href = "launcher.php?port=" + port + "&width=" + width + "&height=" + height + "&gldir=" + gldir + "&db=" + db;
}

function urlOpen(url) {
  window.open(url);
  return false;
}
</script>

</body>
</html>
