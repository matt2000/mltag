<?php
/**
 * @file 
 *   Code for Removing stopwords from content
 * @author Anil Kapur
 *
 */
class Stopwords {
  protected $stopwords;
  /**
   * Function constructor.
   */
  function __construct() {
    $this->stopwords = self::prepare_stopwords();
  }
  /**
   * Prepares a stopwords list using the stopwords_en.inc file
   * and the stopwords defined by the user in the stopwords
   * settings page.
   * @todo extend to all languages for which .inc files are there
   * in the stopwords folder.
   */
  function  prepare_stopwords() {
    module_load_include('inc', 'mltag', 'stopwords/stopwords_en');
    $stopwords_from_file = mltag_stopwords_en();
    $stopwords_user = variable_get('stopwords_textfield', NULL);
    if (trim($stopwords_user)!=NULL || trim($stopwords_user)!='') {
      $stopwords_user = explode(', ', variable_get('stopwords_textfield'));
      $this->stopwords = array_merge($stopwords_from_file, $stopwords_user);
    }
    else {
      $this->stopwords = $stopwords_from_file;
    }
    array_unique($this->stopwords);
    return $this->stopwords;
  }
  /**
   * Remove Stopwords
   */
  function remove_stopwords($tokens) {
    $stopwords = $this->stopwords;
    $new_tokens = array_diff($tokens, $stopwords);
    // no need for the below code as changes added to the Tokenizer itself.
    // special character array.
    return $new_tokens;
  }
  /**
   * Remove Stopwords
   */
  function sentence_remove_stopwords($sentences) {
    $stopwords = $this->stopwords;
    foreach ($sentences as $key => $value) {
      $array_temp = array();
      $array_temp = explode(' ', $value);
      $array_temp = array_diff($array_temp, $stopwords);
      $array_temp = array_values($array_temp);
      $output = '';
      foreach ($array_temp as $key1 => $value1) {
        $word = PorterStemmer::Stem($value1);
        $output .= $word . ' ';
      }
      $sentences["$key"] = trim($output);
    }
    return $sentences;
  }  
} // Class ends.