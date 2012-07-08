<?php

include_once 'stopwords/stopwords_en.inc';
include_once 'includes/content_tag.inc';
include_once 'includes/tokeniser.inc';
include_once 'includes/stopwords1.inc';
include_once 'includes/stemming_porter_algo.inc';

//number of tags to propose
define('tags_num', 30);


/**
 * @todo take care of tokenization of words like 
 * species/subspecies, to-do, he said:hello, hotels/motels
 * at the moment these are taken as a single
 * array element.
 */
// $content stores the original document content
$content = 'KOLKATA: Appreciating an \'overwhelming\' response to her Facebook appeal for supporting former President APJ Abdul Kalam in the presidential election, Mamata Banerjee in another post expressed her thanks on Sunday.

"Thank you all for your overwhelming response, in such a short span of time, for this great cause of nation ... I am sure this united voice will take the issue to the next level", the West Bengal chief minister wrote.

The newest high-value member of the popular social networking website, regretted the erosion of ethics and principles in politics and said public interest had been compromised through the use of money, power and scams.

"It is most unfortunate that politics in our great nation has become murky and values, public interest have been compromised through the use of money, power and scams," she wrote in her official Facebook page.

"Ethical and principled politics has eroded today - this decay has to be reversed at all costs to return to the great traditions of this country".

She also appealed all Indian citizens to raise their voice and be counted to fight against corruption, back-room deal making, Machiavellian manipulations and machinations.

"We should raise our voices and stand by principles - conviction, values and integrity", Banerjee said.

Although known to be tech-savvy, Banerjee had so far stayed away from personally participating in social networking websites despite having courted controversies for being \'intolerant\' of unsavoury comments and cartoons about her.

A source close to her told PTI that she reads tweets regularly and is quite conversant with the online medium.

"This is an age of 360 degree communication where we want to use all channels of communication to spread our party\'s messages. We not only tolerate the social media, we also cherish it", a party spokesperson said.

Ahead of last year\'s Assembly elections, Trinamool had roped in Hotmail co-founder and IT wizard Sabeer Bhatia to help the party campaign better in the cyber world.

Since then, the party has an official Facebook account and a regularly updated website. Trinamool\'s cyber team head and MP Derek O\'Brien, who has more than 78,000 followers on Twitter often uses his tweet to clarify the party\'s stand on various issues. ';

/**Tokenizer class for token generation
 * @return the tokens
 */
$token_object = new Tokenizer($content);
$tokens = $token_object->getTokens();

//extracting sentences from text 
$string_object = new Tokenize_Sentence($content);
$sentences = $string_object->getSentences();
foreach ($sentences as $key => $value)
  echo "<br>".$key.'-->'.$value."<br>";

/** Stopwords class for stopwords removal
 * @return array after removing common stopwords
*/
$stopwords_object = new Stopwords();
$tokens_new = $stopwords_object->remove_stopwords($tokens);

/** Removing stop words from sentences
 *  @return function returns array of sentences 
 *  having no stop words and stemmed words
 */
$stopwords_object_sentence = new Stopwords();
$sentences_new = $stopwords_object_sentence->sentence_remove_stopwords($sentences);
$count = 0; //stores the total number of words in the document
foreach ($sentences_new as $key => $value) {
  echo "<br>".$key.'-->'.$value."<br>";
  $count += count(explode(' ', $value));
}
echo "<br>count = $count <br>";
/** Using porter's algorithm applied stemming
 * @param Tokens
 * @return Array of stemmed words 
 */
$stem = array();
//for mapping the stemmed words to the original word at the end of the algo
$stem_words_map = array(); 
foreach ($tokens_new as $key=>$value){
  $word = PorterStemmer::Stem($value);
  $stem[] = $word;
  $stem_words_map[$value] = $word;
  //echo $value.'-->'.$stem[$value]."<br>";
}
print_r ($stem_words_map);


//pass the array to main tagging algorithm
$tags = content_tag($stem, 1, Null);
$num_frequent_words = count($tags);
echo $num_frequent_words;
//selecting top 20% of the most frequent terms
$frequent_words = array_slice($tags, 0, .2*$num_frequent_words);
$output='';
echo count($frequent_words);
foreach ($frequent_words as $key => $value)
  $output.="$key => $value <br>";
print_r ($output);

/**
 * To calculate expected probability (Pg).
 * Pg =  no. of terms co-occuring with g / Ng(Total terms in doc).
 * $matrix stores frequency of co-occurence freq(w,g) of the chi formula
 * with rows as document terms(w) and columns as frequent terms (g).
 *  
 */
$expected_prob = array();
$matrix = array();
foreach ($frequent_words as $key=>$value) {
  $num = 0;
  foreach ($sentences_new as $key1=>$value1) {
    if(strstr($value1, $key)) {
      $temp = explode(' ',$value1);
      $num += count($temp);
      foreach ($temp as $key3=>$value3) {
        if($value3!=$key) {
        if(!isset($matrix["$value3"]["$key"])) {
          $matrix["$value3"]["$key"] = 0;
        } //inner if ends          
        
        $matrix["$value3"]["$key"] += 1;  
      }//outer if ends      
    } }
  }//inner foreach ends; key is the word
  $expected_prob["$key"] = $num/$count;
}// outer foreach ends 
print_r ($expected_prob);
echo "<br> Matrix-<br>";
foreach($matrix as $key => $value){
  foreach( $value as $key2 => $value2){
    print "$key $key2 => $value2\n<br />\n";
  }
}
/** 
 * code for calculating Wg in the chi formula
 * $matrix_word_count has individual words as row heads and sentence
 * key values as columns heads
 */
$matrix_word_count = array();
foreach ($sentences_new as $key=>$value) {
  $temp = explode(' ', $value);
  $count = 0;
  foreach ($temp as $key1=>$value1) {
    if(strstr($value, $value1)) {
      $matrix_word_count["$value1"]["$key"] = count($temp);
    }
  }
}
echo "<br> Matrix-<br>";
foreach($matrix_word_count as $key => $value){
  foreach( $value as $key2 => $value2){
    print "$key $key2 => $value2\n<br />\n";
  }
}
$nw = array(); //stores the nw value for each word
foreach($matrix_word_count as $word => $value) {
  foreach( $value as $sentence_key => $value2) {
    if(!isset($nw["$word"])) {
      $nw["$word"] = 0; 
    }
     $nw["$word"] += $value2;
  }
}
print_r($nw);
/**
 * calculation of chi value
 */

$chi_matrix = array();
$temp = 0;
$words_array = implode(' ', $sentences_new);
foreach($matrix as $w => $value) {
  foreach( $value as $g => $freq) {
    $temp = $nw["$w"] * $expected_prob["$g"]; 
    $chi_matrix["$w"]["$g"] = (pow(($freq - $temp),2) / $temp) ;    
  }
}
$chi = array(); //stores the chi value for each word
foreach($chi_matrix as $w => $value) {
  foreach( $value as $g => $value2) {
    if(!isset($chi["$w"])) {
      $chi["$w"] = 0;
    }
      $chi["$w"] += $value2;
  } 
}
foreach($chi_matrix as $key => $value){
  foreach( $value as $key2 => $value2){
    print "$key $key2 => $value2\n<br />\n";
  }
}
print_r($chi);
echo "<br>count = ".count($chi)." <br>";
/**
 * calculating X' (chi dash) value
 */
foreach ($chi_matrix as $key=>$value) 
arsort($chi_matrix["$key"]);

//printing again
$chi_max_value = array();
foreach($chi_matrix as $key => $value){
  foreach( $value as $key2 => $value2){
    //print "$key $key2 => $value2\n<br />\n";
    $chi_max_value["$key"] = $value2;
    break; 
  }
}
echo "<br><br>Printing max value<br>";
print_r($chi_max_value);

$chi_dash = array();
foreach ($chi as $word=>$value) {
  $chi_dash["$word"] = $value - $chi_max_value["$word"]; 
}
arsort($chi_dash);
$chi_dash = array_slice($chi_dash, 0, tags_num);
echo "<br><h1><align=\"center\">RESULT</align></h1><br>";
foreach ($chi_dash as $key=>$value) {
  $key = array_search($key, $stem_words_map);
  echo "<br>$key-->$value";
//echo max_chi_value("park", $chi_matrix);
}
echo "<br>count = ".count($chi_dash)." <br>";
  
