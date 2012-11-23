
Leaflet GeoJSON
-----------

A small API module to provide functionality useful
when creating Leaflet maps based on GeoJSON data.

API
-----------

leaflet_geojson_add_bbox_strategy($url).
- Adds a Bounding Box strategy for leaflet maps linking to a specified url.

hook_leaflet_geojson_source_info().
- Allows to specify geojson sources for inclusion in leaflet maps.
  See leaflet_geojson_leaflet_geojson_source_info() for an example that
  provides views_geojson page displays as geojson sources.
  You can actually use those by activating the leaflet_geojson_bean module.

TODO
-----------

- Attach the bbox strategy to a specific map.
- See @todo markers in code.


Maintainers
-----------
- dasjo (Josef Dabernig)

