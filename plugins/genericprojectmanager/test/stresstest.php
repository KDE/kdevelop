<?php

/// prints usage, doesn't exit
function usage() {
    echo "php ./stresstest.php --dir DIRECTORY\n".
         "\n".
         "  DIRECTORY       the directory in which this script\n".
         "                  should randomly touch and move files.\n".
         "\n".
         "Be warned for I will eat thy kittens\n";
}

if ( in_array('--help', $_SERVER['argv']) ) {
    usage();
    exit(0);
}

$i = array_search('--dir', $_SERVER['argv']);
$dir = $i > -1 && $i < $_SERVER['argc'] - 1 ? $_SERVER['argv'][$i+1] : '';

if ( empty($dir) ) {
    usage();
    exit(1);
}

chdir($dir) or die("cannot go to folder $dir\n");

/// our data class with parent pointer so we can safely rename folders
/// and still get proper paths for the children afterwards
class Entry {
    public $p = null;
    public $name;
    /// @return path (including file- / dirname) to this entry
    public function getPath() {
        if (!is_null($this->p)) {
            return $this->p->getPath() . '/' . $this->name;
        } else {
            return $this->name;
        }
    }
}

$dirIt = new RecursiveIteratorIterator(
  new RecursiveDirectoryIterator('.', RecursiveDirectoryIterator::CURRENT_AS_SELF),
  RecursiveIteratorIterator::SELF_FIRST
);

/// we'll grep random entries out of this array
$entries = array();

/// tmp pointers to dir entries
/// [dir => &Entry]
$tmpAssoc = array();

$i = 0;
foreach($dirIt as $file) {
    if ( $file->isDot() ) {
        continue;
    }
    $entries[$i] = new Entry;
    $entries[$i]->name = $file->getFilename();
    $path = $file->getPath();

    // skip .kdev4 folder or files
    if (strpos($path, 'kdev4') != -1) {
        continue;
    }

    if ($path != './') {
        $entries[$i]->p = &$tmpAssoc[$path];
    }
    if ($file->isDir()) {
        $tmpAssoc[$file->getPathname()] = &$entries[$i];
    }
    ++$i;
}

unset($tmpAssoc);

printf("Will move %s files randomly around in %s, do you REALLY want to continue?\n".
       "Type the following if you want to continue: SPARTAAAA ", count($entries), $dir);
$ret = fgets(STDIN, 10);

if (trim($ret) != 'SPARTAAAA') {
    echo "bummer, won't do anything, bye\n";
    exit(2);
}

// microseconds to sleep after move
$sleepTime = 250;

// number of moves until we stop
$moves = 10000;

$moved = 0;
for($moved = 0; $moved < $moves; ++$moved) {
    // get random file to move
    $index = array_rand($entries, 1);
    $entry =& $entries[$index];
    $oldPath = $entry->getPath();
    $newName = $entry->name;
    $lastDot = strrpos($newName, '.');
    if ($lastDot == -1 || $lastDot == 0) {
        $lastDot = strlen($newName);
    } else {
        --$lastDot;
    }
    $newName = substr_replace($newName, rand(), rand() % $lastDot, rand() % $lastDot);
    $entry->name = $newName;
    $newPath = $entry->getPath();
    printf("moving $oldPath to $newPath\n");
    rename($oldPath, $newPath);
    usleep($sleepTime);
}