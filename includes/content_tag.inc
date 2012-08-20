<?php 
/**
 * @file
 * Stores the main tagging and learning algorithms using switch case construct.
 */
//number of tags to propose
define('TAGS_NUMBER', 30);

module_load_include('inc', 'mltag', 'includes/tokeniser');
module_load_include('inc', 'mltag', 'includes/stemming_porter_algo');

function mltag_content_tag($content, $algo_type, $tags_count=TAGS_NUMBER) {
$output = array();  
  switch ($algo_type) {
    case 1: { //This algo works on term frequency
      // original code here  ** dont change
      $token_object = new Tokenizer($content);
      $tokens = $token_object->getTokens();
      $stopwords_object = new Stopwords();
      $tokens = $stopwords_object->remove_stopwords($tokens);
      
      $a = array_count_values($tokens);
      arsort($a);
      $output = $a;
      if ($tags_count != NULL or $tags_count != '')
      $output = array_slice($a, 0, $tags_count, TRUE);
      return $output;
      
      break; 
    } // case 1 ends

    
    case 2: { // word co-occurence statistical algo
      
      /**
       * @todo apply changes from stopwords1.inc to stopwords.inc
       * fit in macro TAGS_NUMBER in the algo      
       * @todo take care of tokenization of words like
       * species/subspecies, to-do, he said:hello, hotels/motels
       * at the moment these are taken as a single
       * array element.
       */
      
      /**
       * Tokenizer class for token generation
       * @return the tokens
      */
      $token_object = new Tokenizer($content);
      $tokens = $token_object->getTokens();
      
      //extracting sentences from text
      $string_object = new Tokenize_Sentence($content);
      $sentences = $string_object->getSentences();
      
      /** 
       * Stopwords class for stopwords removal
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
        $count += count(explode(' ', $value));
      }
      //drupal_set_message($count); 
      /** 
       *Using porter's algorithm applied stemming
       * @param Tokens
       * @return Array of stemmed words
       */
      $stem = array();
      //for mapping the stemmed words to the original word at the end of the algo
      $stem_words_map = array();
      foreach ($tokens_new as $key => $value) {
        $word = PorterStemmer::Stem($value);
        $stem[] = $word;
        $stem_words_map["$value"] = $word;
        //echo $value.'-->'.$stem[$value]."<br>";
      }
      
      $tags = array();      
      $a = array_count_values($stem);
      arsort($a);
      $tags = $a;
      $num_frequent_words = count($tags);
      //drupal_set_message($num_frequent_words);
      //selecting top 20% of the most frequent terms
      $frequent_words = array_slice($tags, 0, .2*$num_frequent_words);
      //drupal_set_message($frequent_words);
      /**
       * To calculate expected probability (Pg).
       * Pg =  no. of terms co-occuring with g / Ng(Total terms in doc).
       * $matrix stores frequency of co-occurence freq(w,g) of the chi formula
       * with rows as document terms(w) and columns as frequent terms (g).
       *
       */
      $expected_prob = array();
      $matrix = array();
      foreach ($frequent_words as $key => $value) {
        $num = 0;
        foreach ($sentences_new as $key1 => $value1) {
          if (strstr($value1, $key)) {
            $temp = explode(' ', $value1);
            $num += count($temp);
            foreach ($temp as $key3 => $value3) {
              if ($value3 != $key) {
                if (!isset($matrix["$value3"]["$key"])) {
                  $matrix["$value3"]["$key"] = 0;
                } //inner if ends
      
                $matrix["$value3"]["$key"] += 1;
              }//outer if ends
            }
          }
        }//inner foreach ends; key is the word
        $expected_prob["$key"] = $num/$count;
      }// outer foreach ends
      
      /**
       * code for calculating Wg in the chi formula
       * $matrix_word_count has individual words as row heads and sentence
       * key values as columns heads
       */
      $matrix_word_count = array();
      foreach ($sentences_new as $key => $value) {
        $temp = explode(' ', $value);
        $count = 0;
        foreach ($temp as $key1 => $value1) {
          //drupal_set_message("$value -> $value1<br>");
          if ($value != "" && $value1 != "") {
          if (strstr($value, $value1)) {
            $matrix_word_count["$value1"]["$key"] = count($temp);
          }
        }}
      }
      
      $nw = array(); //stores the nw value for each word
          foreach ($matrix_word_count as $word => $value) {
            foreach ( $value as $sentence_key => $value2) {
              if (!isset($nw["$word"])) {
                $nw["$word"] = 0;
              }
              $nw["$word"] += $value2;
            }
          }
      
    /**
     * calculation of chi value
     */
      
      $chi_matrix = array();
      $temp = 0;
      $words_array = implode(' ', $sentences_new);
      foreach ($matrix as $w => $value) {
        foreach ( $value as $g => $freq) {
          $temp = $nw["$w"] * $expected_prob["$g"];
          $chi_matrix["$w"]["$g"] = round((pow(($freq - $temp), 2) / $temp), 4) ;
        }
      }
      $chi = array(); //stores the chi value for each word
      foreach ($chi_matrix as $w => $value) {
        foreach ( $value as $g => $value2) {
          if (!isset($chi["$w"])) {
            $chi["$w"] = 0;
          }
          $chi["$w"] += $value2;
        }
      }
      
    /**
     * calculating X' (chi dash) value
     */
      foreach ($chi_matrix as $key => $value)
        arsort($chi_matrix["$key"]);
      
      //printing again
      $chi_max_value = array();
      foreach ($chi_matrix as $key => $value) {
        foreach ( $value as $key2 => $value2) {
          $chi_max_value["$key"] = $value2;
          break;
        }
      }
      
      
      $chi_dash = array();
      foreach ($chi as $word => $value) {
        $chi_dash["$word"] = $value - $chi_max_value["$word"];
      }
      arsort($chi_dash);
      $chi_dash = array_slice($chi_dash, 0, $tags_count, TRUE);
      //echo "<br><h1><align=\"center\">RESULT</align></h1><br>";
      foreach ($chi_dash as $key => $value) {
        unset($chi_dash["$key"]);
        $key = array_search($key, $stem_words_map);
        $chi_dash["$key"] = $value;
        //echo "<br>$key-->$value";
        //echo max_chi_value("park", $chi_matrix);
      }
      //echo "<br>count = ".count($chi_dash)." <br>";
      return $chi_dash;          
      break;
    }// case 2 ends  

    case 3 : {
      /**
       * Takes the content as the input and generates the tokens. The tokens are then matched with 
       * the user_defined tags for pre-existing articles, if any tag matches or exists as a term
       * in the content then the whole set of user defined tags is is added to the 'tags_learn' 
       * array. This follows the assumption that semantically co-related tags co-exist / co-occur
       * with each other.
       * 
       * Then the content is also checked with a ranked list of terms (based on normalized chi square
       * values), that has been derived by the process of training the model.
       * The top 10 ranked terms are added to the 'tags_learn' array, along with the explicit tag block and proposed to the user. 
       *  
       * @todo- take max limit on the tags from the user
       */
      $token_object = new Tokenizer($content);
      $tokens = $token_object->getTokens();
      $stopwords_object = new Stopwords();
      $tokens = $stopwords_object->remove_stopwords($tokens);
      
      $result = db_query("select * from {mltag_learn_tags}");
      $tags_learn = array();
      
      while ($record = $result->fetchObject()) {
        if (($record->user_tags) == '' ||($record->user_tags) == NULL )
          break;
        $user_learned_tags = unserialize($record->user_tags);
        //drupal_set_message(print_r($user_learned_tags, true));
        
        foreach ($user_learned_tags as $key => $value) {
          if (in_array($key, $tokens)) {
            //drupal_set_message($key);
            $tags_learn = array_merge($tags_learn, array_flip($user_learned_tags));
            break;
          }
        }
      }//while ends
      $file = drupal_get_path("module", "mltag") . '/includes/learned_tags.txt';
      $content = file_get_contents($file);
      $arr = unserialize(urldecode($content));
      $arr_keys = array_keys($arr);
      //drupal_set_message(print_r($arr_keys, TRUE));
      $intersection = array_intersect($tokens, $arr_keys);
      $temp = array();
      foreach ($intersection as $key => $word) {
        $temp["$word"] = $arr["$word"];
      }
      arsort($temp);
      //select top 10 ranked matching terms
      $temp = array_slice(array_keys($temp), 0, 10);
      //merge with the learned tag array
      $tags_learn = array_merge($tags_learn, $temp);
      
      //code for combining terms from vocabularies selected in taxonomy module 
      $taxonomy_terms = array();
      $taxonomy_terms = mltag_select_terms_from_taxonomy();
      if (!empty($taxonomy_terms)) {
        $taxonomy_terms = array_intersect($tokens, $taxonomy_terms);
        $tags_learn = array_merge($tags_learn, $taxonomy_terms );
      }     
      
      return array_unique(array_slice($tags_learn, 0 , 20));
    }//case 3 ends
  
  }//switch ends 
 
}//function ends

/**
 * This function returns an array of terms for the vocabularies selected in the configuration
 * page.
 */
function mltag_select_terms_from_taxonomy() {
  
  $output = array();
  if (module_exists('taxonomy')) {
    if (variable_get('user_vocab_list')) {
      $user_vocab_list = variable_get('user_vocab_list');
      $terms = array();
      foreach ($user_vocab_list as $key => $vid) {
        if ($vid == 0) {
          break;
        }
        //$output .= "$key ---> $vid";
        if ($vid != 0) { //0 is received if no vocabulary is selected in configuration page, else vid is received
          $query = db_select('taxonomy_term_data', 't');
          $query
          ->condition('vid', $vid, '=')
          ->fields('t', array('tid', 'name'));
  
          $result = $query->execute();
          $record = $result->fetchAll();
          $num_rows = $result->rowCount();
          //$output .= $num_rows."<br>";
          $i = 0;
          while ($i != $num_rows) {
            $output[] = $record["$i"]->name;
            $i++;
          }
        }
      }
    }
  } //if stmt ends
  return $output;
  //drupal_set_message(print_r($output, TRUE));
}// function ends 