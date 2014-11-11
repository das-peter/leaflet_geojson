<?php

/**
 * @file
 * Views plugin definition.
 */

/**
 * Implements hook_views_data().
 *
 * Support adding leaflet geojson layers to leaflet views.
 */
function leaflet_geojson_views_data() {

  $data['leaflet_geojson']['table']['group'] = t('Leaflet');
  $data['leaflet_geojson']['table']['join'] = array(
    // #global is a special flag which let's a table appear all the time.
    '#global' => array(),
  );

  $data['leaflet_geojson']['geojson_layer'] = array(
    'title' => t('GeoJSON Layer'),
    'help' => t('Attach a leaflet GeoJSON Layer to this view.'),
    'area' => array(
      'handler' => 'views_handler_area_leaflet_geojson',
    ),
    'theme' => 'leaflet_geojson_geojson_layer',
  );

  return $data;
}
