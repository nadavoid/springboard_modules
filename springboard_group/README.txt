Springboard Groups integrates Organic Groups with Springboard.

It does this by both limiting and extending the functionality of the Organic Groups module.

Limiting:

Organic groups allows almost any entity type to be a group. Springboard Groups only
interacts with a single group content type, the "springboard_group" content type.

Organic groups allows almost any entity type to be a group content. Springboard Groups disallows
some specific entities from being groupable within Springboard Groups.

Organic Groups' group selector widget is complex, with multiple input fields and multiple select options.
 Springboard Groups alters the widget to be a single input field, with a choice of either multiple or
 single select options. Single select options are forced on some entity types which cannot belong to
 multiple groups.

Organic groups allows for a myriad of configuration options for group types, group content,
group roles, group fields and group permissions. Springboard Groups provides a stripped down admin UI,
and menu alters the native OG configuration pages to be hidden with a custom access permission. Springboard
Groups does not provide a UI for custom fields, multiple group types, etc.

Extending:

Springboard Groups adds a Group Context selector which appears on non-node pages, such as
the Springboard Dashboard view, which allows content listings on those pages to be filtered by a group
selection. Filtering is done via views query and page alter hooks. Filtering for users who belong to a
single group is done in the background, without a context selector.

Springboard Groups integrates with the Springboard Advocacy API, allowing remote content on the API server
to belong to Springboard Groups, and individual Springboard Groups to have custom Advocacy subscription settings.

Springboard Groups integrates with email wrappers, page wrappers, payment methods, and webform ab tests,
and validates group ownership of these nodes in a two-way, back-referenced manner which prevents the creation
of conflicting ownership configurations between a grouped webform and a grouped asset.

Springboard Groups improves the node-specific Group Membership UI by altering the
navigation structure to be more compact.

Springboard Groups provides a bulk-add members option.

On installation:

The springboard group install function creates an “affiliate editor” role,
grants it a small set of springboard permissions, and also confers a minimal
set of groups-related permissions to the existing springboard roles, i.e.,
makes the sbadmin role a groups administrator, and confers access to the groups
dashboard to the other roles. Inside the group, there are two roles by default,
member and administrator member, and each of those roles gets a small set of
in-group permissions by default.

None of the content types or entities are group-enabled by default. When you enable groups on a type,
the configuration is updated to make any Springboard Views used by that type group-aware.

Technical Details:

In standard organic groups, group audience fields have an entity reference selection plugin "OgSelectionHandler".
In Springboard Groups, when you enable grouping of an entity type in the alternative admin UI, the group audience
field gets a custom entity reference selection handler "OgSpringboardOgSelectionHandler". This handler makes it possible
to replace the OG "complex" (two fields) group selection field with a simple single field.

Adding the group field through the alternative UI also creates a machine name for the group field:
"og_group_ref_[bundle_name]." The default behavior in OG is to share a single machine name "og_group_ref". It also
makes "springboard_group" the only reference-able bundle for the reference field.
