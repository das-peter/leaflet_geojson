(function ($) {

  Drupal.leafletBBox = {

    map: null,
    markerGroup: null,
    overlays: {},

    onMapLoad: function(map) {
      Drupal.leafletBBox.map = map;
      Drupal.leafletBBox.markerGroup = new Array();

      // Intialize empty layers and associated controls.
      var layer_count = 0;
      $.each(Drupal.settings.leafletBBox, function(key, value) {
        if (typeof value.url !== 'undefined') {
          // Add empty layers.
          Drupal.leafletBBox.markerGroup[key] = new L.LayerGroup();
          Drupal.leafletBBox.markerGroup[key].addTo(map);

          // Connect layer controls to layer data.
          Drupal.leafletBBox.overlays[value.title]
            = Drupal.leafletBBox.markerGroup[key];

          layer_count++;
        }
      });

      // If we have more than one data layer, add the control.
      // @TODO: figure out how to interact with base map selection.
      if (layer_count > 1) {
        L.control.layers(null, Drupal.leafletBBox.overlays).addTo(map);
      }

      // Loading a map is the same as moving/zooming.
      map.on('moveend', Drupal.leafletBBox.moveEnd);
      Drupal.leafletBBox.moveEnd();
    },

    moveEnd: function(e) {
      var map = Drupal.leafletBBox.map;
      if (!map._popup) {
        // Rebuild the bounded GeoJSON layers.
        $.each(Drupal.settings.leafletBBox, function(layer_key, layer_info) {
          if (typeof layer_info.url !== 'undefined') {
            Drupal.leafletBBox.makeGeoJSONLayer(map, layer_info, layer_key);
          }
        });
      }
    },

    makeGeoJSONLayer: function(map, info, layer_key) {
      var url = typeof info.url !== 'undefined' ? info.url : Drupal.settings.leafletBBox.url;

      var bbox_arg_id = ('bbox_arg_id' in Drupal.settings.leafletBBox) ?
        Drupal.settings.leafletBBox.bbox_arg_id : 'bbox';

      // Add bbox and zoom parameters as get params.
      url += "?" + bbox_arg_id +"=" + map.getBounds().toBBoxString();
      url += "&zoom=" + map.getZoom();

      // Append any existing query string (respect exposed filters).
      if (window.location.search.substring(1) != '') {
        url += "&" + window.location.search.substring(1);
      }

      // Make a new GeoJSON layer.
      $.getJSON(url, function(data) {
        var geojsonLayer = new L.GeoJSON(data, Drupal.leafletBBox.geoJSONOptions);
        Drupal.leafletBBox.markerGroup[layer_key].clearLayers();
        Drupal.leafletBBox.markerGroup[layer_key].addLayer(geojsonLayer);

        // Connect the layer control to the new data.
        Drupal.leafletBBox.overlays[info.title] = Drupal.leafletBBox.markerGroup[layer_key];
      });
    }
  };

  Drupal.leafletBBox.geoJSONOptions = {

    onEachFeature: function(feature, layer) {
      if (feature.properties) {
        if (feature.properties.description) {
          layer.bindPopup(feature.properties.description);
        } else if (feature.properties.name) {
          layer.bindPopup(feature.properties.name);
        }
      }
    }

  };

  // Insert map.
  $(document).bind('leaflet.map', function(e, map, lMap) {
    Drupal.leafletBBox.onMapLoad(lMap);
  });

})(jQuery);
