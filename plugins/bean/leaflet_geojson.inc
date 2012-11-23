<?php
/**
 * @file
 * Listing bean plugin.
 */

class LeafletGeoJSON extends BeanPlugin {
  /**
   * Declares default block settings.
   */
  public function values() {
    $values = array(
      'settings' => array(
        'map' => NULL,
        'height' => '400',
        'source' => NULL,
      ),
    );
    return array_merge(parent::values(), $values);
  }
  /**
   * Builds extra settings for the block edit form.
   */
  public function form($bean, $form, &$form_state) {
    $form = array();
    $form['settings'] = array(
      '#type' => 'fieldset',
      '#tree' => 1,
      '#title' => t('Settings'),
    );
    // Choose a geojson source.
    $source_options = $this->getSourceOptions();
    $default_source = isset($bean->settings['source']) ? $bean->settings['source'] : key($source_options);
    $form['settings']['source'] = array(
      '#type' => 'select',
      '#title' => t('GeoJSON source'),
      '#options' => $source_options,
      '#default_value' => $default_source,
      '#required' => TRUE,
    );
    // Choose a map preset.
    $map_options = $this->getMapOptions();
    $default_map = isset($bean->settings['map']) ? $bean->settings['map'] : key($map_options);
    $form['settings']['map'] = array(
      '#title' => t('Leaflet map'),
      '#type' => 'select',
      '#options' => $map_options,
      '#default_value' => $default_map,
      '#required' => TRUE,
    );
    $default_height = isset($bean->settings['height']) ? $bean->settings['height'] : 400;
    $form['settings']['height'] = array(
      '#title' => t('Map height'),
      '#type' => 'textfield',
      '#field_suffix' => t('px'),
      '#size' => 4,
      '#default_value' => $default_height,
      '#required' => FALSE,
    );
    return $form;
  }

  /**
   * Displays the bean.
   */
  public function view($bean, $content, $view_mode = 'default', $langcode = NULL) {
    // Prepare leaflet map settings.
    $map = leaflet_map_get_info($bean->settings['map']);
    $height = $bean->settings['height'];

    // Prepare source and fetch json.
    $source = leaflet_geojson_get_source($bean->settings['source']);
    /*
    $json = $this->getGeoJSON($source);
    $features = array(
      array(
        'type' => 'json',
        'json' => $json,
      )
    );
    */
    $features = array();

    $content['map'] = array(
      '#markup' => leaflet_render_map($map, $features, $height . 'px'),
    );

    if ($source['useBBox']) {
      drupal_add_js(
        drupal_get_path('module', 'leaflet_geojson') . '/leaflet.bbox.js',
        array(
          // Add our script after leaflet.drupal.js.
          'weight' => 5
        )
      );
      // Add custom settings.
      $settings = array(
        'url' => $source['url'],
      );
      drupal_add_js(array('leafletBBox' => $settings), 'setting');
    }
    return $content;
  }

  protected function getMapOptions() {
    $map_options = array();
    foreach (leaflet_map_get_info() as $key => $map) {
      $map_options[$key] = t($map['label']);
    }
    return $map_options;
  }

  protected function getSourceOptions() {
    $sources = leaflet_geojson_sources();
    $source_options = array();
    foreach ($sources as $id => $source) {
      $source_options[$id] = $source['title'];
    }
    return $source_options;
  }

  protected function getGeoJSON($source) {
    /*
     * @todo: doesn't work because views_geojson directly outputs and terminates the request.
    if ($source['type'] == 'views_geojson') {
      $view = views_get_view($source['view']);
      $output = $view->execute_display($source['view_display']);
    }
    */
    $result = drupal_http_request($source['url']);
    if (in_array( $result->code, array(200, 304))) {
      $json_string = (isset($result->data)) ? $result->data : '';
      return drupal_json_decode($json_string);
    }
    else {
      // Error $result->code
    }
  }
}
