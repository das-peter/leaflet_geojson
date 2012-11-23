(function ($) {

  Drupal.leafletBBox = {

    map: null,
    markerGroup: null,

    onMapLoad: function(event) {
      var map = this;
      Drupal.leafletBBox.map = map;

      Drupal.leafletBBox.markerGroup = new L.LayerGroup();
      Drupal.leafletBBox.markerGroup.addTo(map);

      map.on('moveend', Drupal.leafletBBox.moveEnd);
      Drupal.leafletBBox.moveEnd();
    },

    moveEnd: function(e) {
      var map = Drupal.leafletBBox.map;
      Drupal.leafletBBox.makeGeoJSONLayer(map);
    },

    makeGeoJSONLayer: function(map, url) {
      url = typeof url !== 'undefined' ? url : Drupal.settings.leafletBBox.url;

      // Add bbox and zoom parameters as get params.
      url += "?bbox=" + map.getBounds().pad(0.05).toBBoxString();
      url += "&zoom=" + map.getZoom();

      $.getJSON(url, function(data) {
        //New GeoJSON layer
        var geojsonLayer = new L.GeoJSON(data, Drupal.leafletBBox.geoJSONOptions);
        Drupal.leafletBBox.markerGroup.clearLayers();
        Drupal.leafletBBox.markerGroup.addLayer(geojsonLayer);
      });
    }

  };

  Drupal.leafletBBox.geoJSONOptions = {

    onEachFeature: function(featureData, layer) {
      var popupText = featureData.properties.name;
      layer.bindPopup(popupText);
    },

    pointToLayer: function(featureData, latlng) {
      var icon = new L.NumberedDivIcon({number: featureData.cluster_items || 1});
      lMarker = new L.Marker(latlng, {icon:icon});
      return lMarker;
    }

  };

  // Inject into leaflet initialize.
  // @todo: there should be a nicer way to do that?
  _leaflet_bbox_old_leaflet_initialize = L.Map.prototype.initialize;
  L.Map.include({
    initialize: function(/*HTMLElement or String*/ id, /*Object*/ options) {
      _leaflet_bbox_old_leaflet_initialize.apply(this, [id, options]);
      this.on('load', Drupal.leafletBBox.onMapLoad, this);
    }
  });

})(jQuery);
