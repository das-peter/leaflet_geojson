<?php

/**
 * @file
 * Views area handlers for leaflet geojson.
 */

/**
 * Allows us to attach multiple layers to a leaflet views map.
 */
class views_handler_area_leaflet_geojson extends views_handler_area {

  /**
   * List with the names of supported style plugins.
   * @var array
   */
  protected $supportedStylePlugins = array('leaflet');

  /**
   * Stores the settings to be added by geojson_leaflet_map_prebuild_alter().
   *
   * Associative array of settings keyed by the map id.
   *
   * @see views_handler_area_leaflet_geojson::geojson_leaflet_map_prebuild_alter()
   *
   * @var array
   */
  protected static $mapSettings = array();

  /**
   * Checks if the given $plugin is supported by this area handler.
   *
   * @param string $style_plugin
   *   The style plugin to check.
   *
   * @return bool
   *   Whether or not the area supports the given plugin.
   */
  public function supportsStylePlugin($style_plugin) {
    return in_array($style_plugin, $this->supportedStylePlugins);
  }

  /**
   * {@inheritdoc}
   */
  public function option_definition() {
    $options = parent::option_definition();
    $options['add_layer_control'] = array('default' => FALSE, 'bool' => TRUE);
    $options['leaflet_geojson_source'] = array('default' => '');
    return $options;
  }

  /**
   * Provide options for the integration into other leaflet maps.
   */
  public function options_form(&$form, &$form_state) {
    // Check if this views display is supported actually.
    $supported_styles = array('leaflet');
    if (!$this->supportsStylePlugin($this->view->display_handler->get_option('style_plugin'))) {
      $supported_style_names = array();
      foreach ($this->supportedStylePlugins as $supported_style) {
        if ($plugin = views_get_plugin('style', $supported_style)) {
          $supported_style_names[] = $plugin->plugin_title();
        }
      }
      $form['warning'] = array(
        '#type' => 'item',
        '#title' => t('Attention'),
        '#markup' => t('This area only supports views with the output format: %plugins', array('%plugins' => implode(' ' . t('or') . ' ', $supported_style_names))),
      );
      return;
    }

    // Fetch default options.
    parent::options_form($form, $form_state);

    $form['leaflet_geojson_source'] = array(
      '#type' => 'select',
      '#required' => 'select',
      '#title' => t('Select the source for the layer'),
      '#description' => t('Currently works just with sources that have a bbox argument handler.'),
      '#default_value' => $this->options['leaflet_geojson_source'],
      '#options' => $this->getSourceOptions(),
    );

    $form['add_layer_control'] = array(
      '#type' => 'checkbox',
      '#title' => t('Add a layer control for this source'),
      '#default_value' => !empty($this->options['add_layer_control']),
    );
  }

  /**
   * Returns the available source options.
   *
   * @return array
   *   Options array.
   */
  protected function getSourceOptions() {
    $sources = leaflet_geojson_source_get_info(NULL, TRUE);
    $source_options = array();
    foreach ($sources as $id => $source_info) {
      // Currently only sources with bbox enabled will work.
      if (!empty($source_info['bbox'])) {
        $source_options[$id] = $source_info['title'];
      }
    }
    return $source_options;
  }

  /**
   * {@inheritdoc}
   *
   * @TODO Adjust leaflet views so that we can inject data. E.g.
   * leaflet_views_plugin_style could store the build result from
   * leaflet_build_map() in a property by reference so we can modify it.
   */
  public function render($empty = FALSE) {
    if (!$empty || !empty($this->options['empty'])) {
      $map_id = $this->view->style_plugin->getMapId();
      $leaflet_layer_id = drupal_html_id('leaflet-geojson-' . $map_id);

      // Add GeoJSON Layer.
      $source_info = leaflet_geojson_source_get_info($this->options['leaflet_geojson_source']);
      $source_info['leaflet_map_id'] = $map_id;
      $source_info['leaflet_layer_id'] = $leaflet_layer_id;
      if (!empty($source_info['bbox'])) {
        leaflet_geojson_add_bbox_strategy($source_info, $leaflet_layer_id);
      }

      // Fetch and add the first data.
      $json = leaflet_geojson_fetch_json_from_source($source_info);
      $json['group'] = TRUE;
      $json['label'] = 'Group Label';
      $json['leaflet_id'] = $leaflet_layer_id;
      views_handler_area_leaflet_geojson::$mapSettings[$map_id]['features'][] = $json;

      // Add layer control if enabled.
      if (!empty($this->options['add_layer_control'])) {
        views_handler_area_leaflet_geojson::$mapSettings[$map_id]['map']['settings']['layerControl'] = TRUE;
      }
    }
    return '';
  }

  /**
   * Inject our settings into the leaflet map settings.
   *
   * Called by leaflet_geojson_leaflet_map_prebuild_alter()
   *
   * @see leaflet_geojson_leaflet_map_prebuild_alter()
   */
  public static function geojson_leaflet_map_prebuild_alter(&$settings) {
    $map_id = $settings['mapId'];
    if (isset(self::$mapSettings[$map_id])) {
      $settings = drupal_array_merge_deep($settings, self::$mapSettings[$map_id]);
    }
  }
}
