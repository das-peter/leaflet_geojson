<?php
/**
 * @file
 * Pane for multiple data layer leaflet maps.
 */

$plugin = array(
  'title' => t('Leaflet GeoJSON map'),
  'description' => t('Display a Leaflet map driven by GeoJSON feeds.'),
  'category' => array(t('Leaflet GeoJSON')),
  'single' => TRUE,
  'defaults' => array(),
  'admin info' => 'leaflet_geojson_map_pane_admin_info',
  'render callback' => 'leaflet_geojson_map_pane_render',
  'edit form' => 'leaflet_geojson_map_pane_edit_form',
);

/**
 * Callback for page_manager admin UI information.
 */
function leaflet_geojson_map_pane_admin_info($subtype, $conf, $contexts) {
  if (!empty($conf)) {
    $block = new stdClass();
    $block->title = $conf['override_title'] ? $conf['override_title_text'] : '';

    $layer_count = count($conf['map_settings']['info']['data']);
    $admin_info = t('@map_base (@count data @layers)', array(
      '@map_base' => $conf['map_settings']['base'],
      '@count' => $layer_count,
      '@layers' => format_plural($layer_count, 'layer', 'layers'),
    ));
    $block->content = $admin_info;

    return $block;
  }
}

/**
 * Callback to render the pane.
 */
function leaflet_geojson_map_pane_render($subtype, $conf, $panels_args, $context) {
  // Build the pane.
  $pane = new stdClass();
  $pane->module = 'leaflet_geojson';

  // This title can/should be overriden in the page manager UI.
  $pane->title = 'Leaflet Map';

  $pane->content = leaflet_geojson_map_pane_render_layered_map($conf);

  return $pane;
}

/**
 * Helper function to generate the map markup based on the pane config.
 */
function leaflet_geojson_map_pane_render_layered_map($conf) {

  // Gather information about the leaflet base and data layers.
  $map_base_info = leaflet_map_get_info($conf['map_settings']['base']);
  $data_layers_info = array();
  foreach ($conf['map_settings']['info']['data'] as $layer_idx => $layer_machine_name) {
    $data_layers_info[$layer_idx]
      = leaflet_geojson_source_get_info($layer_machine_name);
  }

  // We are not currently supporting mixed bounding, i.e. if any one layer
  // is not bounded, then all layers will be fetched non-bounded. This will
  // have serious performance impact at large scale.
  $all_bounded = TRUE;
  foreach ($data_layers_info as $layer_idx => $layer_info) {
    if (!isset($layer_info['bbox'])) {
      $all_bounded = FALSE;
      break;
    }
  }

  $feature_layers = array();
  if ($all_bounded) {

    // Ensure the map center is non-empty for bbox.
    if (empty($map_base_info['center'])) {
      $map_base_info['center'] = array('lon' => 0,'lat' => 0);
    }
    // Add the bounding (bbox) script.
    leaflet_geojson_add_bbox_strategy($data_layers_info);

    // If we are using geocluster, add the javascript.
    if (module_exists('geocluster')) {
      drupal_add_js(
        drupal_get_path('module', 'geocluster') . '/js/geocluster.leaflet.bbox.js',
        array(
          // Make sure geocluster's weight is loaded after leaflet.bbox
          'weight' => 100,
        )
      );
    }
  }
  else {
    /* @TODO: non-bounded data fetch */
  }


  // Apply any overrides of natural center and zoom.
  if (!empty($conf['map_settings']['override_zoom_center'])) {
    if (!empty($conf['map_settings']['custom_zoom_center']['zoom'])) {
      $map_base_info['settings']['zoom']
        = $conf['map_settings']['custom_zoom_center']['zoom'];
    }
    if (!empty($conf['map_settings']['custom_zoom_center']['center']['lat']) && !empty($conf['map_settings']['custom_zoom_center']['center']['lon'])) {
      $map_base_info['center'] = array(
        'lat' => $conf['map_settings']['custom_zoom_center']['center']['lat'],
        'lon' => $conf['map_settings']['custom_zoom_center']['center']['lon'],
      );
    }
  }

  // Allow other modules to alter the map data.
  drupal_alter('leaflet_geojson_map_pane', $map_base_info, $feature_layers);

  $map = leaflet_build_map(
    $map_base_info, $feature_layers, $conf['map_settings']['height'] . 'px'
  );
  return render($map);
}

/**
 * Helper function to retrieve a Views GeoJSON feed without bounding.
 */
function leaflet_geojson_map_pane_fetch_json($source_info) {
  $result = drupal_http_request($source_info['url']);
  if (in_array($result->code, array(200, 304))) {
    $json_string = (isset($result->data)) ? $result->data : '';
    return drupal_json_decode($json_string);
  }
  else {
    return NULL;
  }
}

/**
 * Edit form for the pane's settings.
 */
function leaflet_geojson_map_pane_edit_form($form, &$form_state) {
  $conf = $form_state['conf'];

  // Build base layer selector.
  $base_options = array();
  foreach (leaflet_map_get_info() as $key => $map) {
    $base_options[$key] = t($map['label']);
  }
  // The default selection is the first one, or the previously selected one.
  $default_base = key($base_options);
  if (isset($conf['map_settings']['base'])) {
    $default_base = $conf['map_settings']['base'];
  }
  $form['map_settings']['base'] = array(
    '#title' => t('Leaflet base layer'),
    '#type' => 'select',
    '#options' => $base_options,
    '#default_value' => $default_base,
    '#required' => TRUE,
    '#description' => t(
        'Select the Leaflet base layer (map style) that will display the data.'
    ),
  );

  // Provide some UI help for setting up multi-layer maps.
  $data_layers_description
    = t('Choose one or more GeoJSON sources that will provide the map data. ');
  $data_layers_description
    .= t('If more than one source  is selected, a layer control will appear on the map. ');
  $data_layers_description
    .= t('Views GeoJSON page displays are automatically exposed here.');

  // Build the data layers selector.
  $form['map_settings']['info'] = array(
    '#type' => 'fieldset',
    '#tree' => TRUE,
    '#title' => 'Views GeoJSON Data Layers',
    '#collapsible' => TRUE,
    '#collapsed' => FALSE,
    '#description' => $data_layers_description,
  );

  // Grab the available layers.
  $sources = leaflet_geojson_source_get_info(NULL, TRUE);
  $source_options = array();
  foreach ($sources as $id => $source) {
    $source_options[$id] = $source['title'];
  }

  // Figure out if we have any data layers yet, and set the layer count..
  if (empty($form_state['layer_count'])) {
    // During creation, we wont have any data yet, so only one layer.
    if (!isset($conf['map_settings']['info']['data'])) {
      $form_state['layer_count'] = 1;
    }
    else {
      // During edit, we'll have one or more layers, so count.
      $form_state['layer_count'] = count($conf['map_settings']['info']['data']);
    }
  }

  // Build the number of layer selections indicated by layer_count.
  for ($layer_index = 1; $layer_index <= $form_state['layer_count']; $layer_index++) {
    $default_layer_source = key($source_options);
    if (isset($conf['map_settings']['info']['data']['leaflet_' . $layer_index])) {
      $default_layer_source
        = $conf['map_settings']['info']['data']['leaflet_' . $layer_index];
    }
    $form['map_settings']['info']['data']['leaflet_' . $layer_index] = array(
      '#type' => 'select',
      '#title' => t('GeoJSON layer source'),
      '#options' => $source_options,
      '#default_value' => $default_layer_source,
      '#required' => ($layer_index == 1),
    );
  }

  // Provide an "Add another layer" button.
  $form['map_settings']['info']['add_layer'] = array(
    '#type' => 'submit',
    '#value' => t('Add another layer'),
    '#submit' => array('leaflet_geojson_map_pane_add_layer'),
  );

  // Provide a "Remove" button for latest selected layer.
  if ($form_state['layer_count'] > 1) {
    $form['map_settings']['info']['remove_layer'] = array(
      '#type' => 'submit',
      '#value' => t('Remove last layer'),
      '#submit' => array('leaflet_geojson_map_pane_remove_last_layer'),
    );
  }

  // Leaflet wants a height in the call to the render function.
  $default_height = isset($conf['map_settings']['height']) ? $conf['map_settings']['height'] : 400;
  $form['map_settings']['height'] = array(
    '#title' => t('Map height'),
    '#type' => 'textfield',
    '#field_suffix' => t('px'),
    '#size' => 4,
    '#default_value' => $default_height,
    '#required' => FALSE,
    '#description' => t("Set the map height in pixels."),
  );

  // Optionally override natural center and zoom.
  $default_override_zoom_center = isset($conf['map_settings']['override_zoom_center']) ? $conf['map_settings']['override_zoom_center'] : FALSE;
  $form['map_settings']['override_zoom_center'] = array(
    '#type' => 'checkbox',
    '#title' => 'Override natural center and zoom placement',
    '#default_value' => $default_override_zoom_center,
    '#description' => t("Map will auto zoom and center based on the data. Check this box to customize the zooom and center"),
  );
  $form['map_settings']['custom_zoom_center'] = array(
    '#type' => 'fieldset',
    '#title' => 'Zoom and Center',
    '#tree' => TRUE,
    '#states' => array(
      'visible' => array(
        ':input[name="override_zoom_center"]' => array('checked' => TRUE),
      ),
    ),
  );
  $default_zoom = isset($conf['map_settings']['custom_zoom_center']['zoom']) ? $conf['map_settings']['custom_zoom_center']['zoom'] : 1;
  $form['map_settings']['custom_zoom_center']['zoom'] = array(
    '#title' => t('Zoom'),
    '#type' => 'textfield',
    '#size' => 20,
    '#default_value' => $default_zoom,
    '#required' => FALSE,
  );
  $form['map_settings']['custom_zoom_center']['center'] = array(
    '#type' => 'fieldset',
    '#title' => 'Map center',
    '#tree' => TRUE,
    '#description' => t("Provide a default map center especially when using the bounding box strategy."),
  );
  $default_center = isset($conf['map_settings']['custom_zoom_center']['center']) ? $conf['map_settings']['custom_zoom_center']['center'] : array('lon' => 0, 'lat' => 0);
  $form['map_settings']['custom_zoom_center']['center']['lon'] = array(
    '#title' => t('Center longitude'),
    '#type' => 'textfield',
    '#size' => 20,
    '#default_value' => $default_center['lon'],
    '#required' => FALSE,
  );
  $form['map_settings']['custom_zoom_center']['center']['lat'] = array(
    '#title' => t('Center latitude'),
    '#type' => 'textfield',
    '#size' => 20,
    '#default_value' => $default_center['lat'],
    '#required' => FALSE,
  );

  return $form;
}

/**
 * Helper function to add another views_geojson layer to the map data.
 */
function leaflet_geojson_map_pane_add_layer($form, &$form_state) {
  // Increment the count and force a rebuild.
  $form_state['layer_count']++;
  $form_state['rebuild'] = TRUE;
}

/**
 * Helper function to remove the last views_geojson layer from the map.
 */
function leaflet_geojson_map_pane_remove_last_layer($form, &$form_state) {
  // Decrement the count and force a rebuild.
  $form_state['layer_count']--;
  $form_state['rebuild'] = TRUE;
}

/**
 * Submit handler just puts non-empty values into $form_state['conf'].
 */
function leaflet_geojson_map_pane_edit_form_submit($form, &$form_state) {
  foreach (element_children($form['map_settings']) as $key) {
    if (!empty($form_state['values'][$key])) {
      $form_state['conf']['map_settings'][$key] = $form_state['values'][$key];
    }
  }
}
