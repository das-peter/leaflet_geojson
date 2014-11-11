(function ($) {

  Drupal.leafletBBox = {

    // Stores the instances of bbox handling.
    instances: [],

    onMapLoad: function(leaflet_map_id, map) {

      if (typeof(Drupal.settings.leafletBBox[leaflet_map_id]) != 'undefined') {

        Drupal.leafletBBox.instances[leaflet_map_id] = {
          map: map,
          markerGroup: null
        };

        map.eachLayer(function (layer) {
          if (layer._leaflet_id == Drupal.settings.leafletBBox[leaflet_map_id].leaflet_layer_id) {
            Drupal.leafletBBox.instances[leaflet_map_id].markerGroup = layer;
          }
        });

        if (Drupal.leafletBBox.instances[leaflet_map_id].markerGroup == null) {
          Drupal.leafletBBox.instances[leaflet_map_id].markerGroup = new L.LayerGroup();
          Drupal.leafletBBox.instances[leaflet_map_id].markerGroup.addTo(map);
          Drupal.leafletBBox.moveEnd();
        }
        map.on('moveend', function(e) { Drupal.leafletBBox.moveEnd(e, leaflet_map_id) });
      }
    },

    moveEnd: function(e, leaflet_map_id) {
      var map = Drupal.leafletBBox.instances[leaflet_map_id].map;
      if (!map._popup) {
        Drupal.leafletBBox.makeGeoJSONLayer(leaflet_map_id);
      }
    },

    makeGeoJSONLayer: function(leaflet_map_id, url) {
      var instance = Drupal.leafletBBox.instances[leaflet_map_id];
      var map = instance.map;
      var settings = Drupal.settings.leafletBBox[leaflet_map_id];
      url = typeof url !== 'undefined' ? url : settings.url;

      var bbox_arg_id = ('bbox_arg_id' in settings) ?
        settings.bbox_arg_id : 'bbox';

      // Add bbox and zoom parameters as get params.
      url += "?" + bbox_arg_id +"=" + map.getBounds().toBBoxString();
      url += "&zoom=" + map.getZoom();

      $.getJSON(url, function(data) {
        //New GeoJSON layer
        var geojsonLayer = new L.GeoJSON(data, Drupal.leafletBBox.geoJSONOptions);
        instance.markerGroup.clearLayers();
        instance.markerGroup.addLayer(geojsonLayer);
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

  $(document).bind('leaflet.map', function(e, map, lMap){
    Drupal.leafletBBox.onMapLoad(map.map_id, lMap);
  });

})(jQuery);
