<?php

/**
 * @file
 * Hooks provided by the Leaflet GeoJSON module.
 */

/**
 * @addtogroup hooks
 * @{
 */

/**
 * Defines one or more Leaflet GeoJSON sources.
 *
 * Note: The ids should be valid PHP identifiers.
 *
 * @see hook_leaflet_geojson_source_info_alter()
 *
 * @return array
 *   An associative array of sources, keyed by a unique
 *   identifier and containing associative arrays with the following keys:
 *   - leaflet_map_id: The id of the map this source belongs to.
 *   - title: A human readable title for the source.
 *   - url: The GeoJSON source url.
 *   - bbox: (optional) Set to TRUE to activate the Bounding Box strategy.
 *   - bbox_arg_id: (optional) Specify the bbox argument identifier.
 *       Defaults to 'bbox'.
 *   - type: (optional) The type of source, for example views_geojson.
 *   - group_id: (optional) Id of the layer group the features are assigned to.
 *   - group_label: (optional) Label of the group for e.g. the layer control.
 *   - group_layer_control: (optional) If enabled the layer control of the map
 *     is enabled.
 *   - ... (optional) further parameters specific to the source type.
 */
function hook_leaflet_geojson_source_info() {
  $sources = array();
  $sources['simple_source'] = array(
    'title' => 'My Source',
    'url' => 'http://example.com',
  );
  $sources['views_geojson_source'] = array(
    'leaflet_map_id' => 'points_of_interest',
    'title' => 'My Source',
    'url' => 'http://example.com',
    'bbox' => TRUE,
    'bbox_arg_id' => 'bbox',
    'type' => 'views_geojson',
    'view' => 'view_name',
    'view_display' => 'view_display_name',
    'group_id' => 'views_geojson_source_group_layer',
    'group_label' => t('Points of interest'),
    'group_layer_control' => TRUE,
  );
  return $sources;
}

/**
 * Alter the Leaflet GeoJSON source info.
 *
 * @param array $infos
 *   The source info array, keyed by source identifier.
 *
 * @see hook_leaflet_geojson_source_info()
 */
function hook_leaflet_geojson_source_info_alter(array &$infos) {
  $infos['simple_source']['url'] = 'http://somewhere.else';
}

/**
 * Alter the Leaflet GeoJSON settings just before they are used.
 *
 * @param array $settings
 *   The source info array, keyed by source identifier.
 *
 * @see hook_leaflet_geojson_source_info()
 */
function leaflet_geojson_add_bbox_strategy_alter(array &$settings) {
  // Add your own JS or adjust settings.
}
