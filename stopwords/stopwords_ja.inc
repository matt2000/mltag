<?php

// Japanese Stopwords
function mltag_stopwords_ja() {
	$sp = 'あそこ, こちら, 貴方方, あの人, あのかた, あります, おります, います, それで, しかし';
  
  $list = explode(', ', $sp);
  return array('ja' => $list);
}