<?php 
/**
 * Define various algorithms used throughout this project
 */
function hook_mltag_custom_algorithms() {
  return array(
      'mymodule_first_algo' => array(
          'label' => 'My first awesome algorithm',
          'description' => 'This algorithm selects terms based on a ranking of their awesomeness.',
      ),
      'mymodule_other_algo' => array('label' => 'My other mysterious algorithm',
      ),
  );

}
