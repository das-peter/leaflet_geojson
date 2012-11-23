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
    $source_info = leaflet_geojson_source_get_info($bean->settings['source']);
    $features = array();

    $context = array(
      'map' => &$map,
      'source_info' => &$source_info,
      'bean' => &$bean,
    );
    drupal_alter('leaflet_geojson_bean_view_features', $features, $context);

    $content['map'] = array(
      '#markup' => leaflet_render_map($map, $features, $height . 'px'),
    );
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
    $sources = leaflet_geojson_source_get_info(NULL, TRUE);
    $source_options = array();
    foreach ($sources as $id => $source) {
      $source_options[$id] = $source['title'];
    }
    return $source_options;
  }

}
