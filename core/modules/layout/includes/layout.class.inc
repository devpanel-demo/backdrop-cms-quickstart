<?php
/**
 * @file
 * Class for loading, modifying, and executing a layout.
 */
class Layout {
  /**
   * The name of the layout.
   *
   * @var string
   */
  var $name = '';

  /**
   * The human readable name of the layout.
   *
   * @var string
   */
  var $title = '';

  /**
   * The description of the view, which is used only in the interface.
   *
   * @var string
   */
  var $description;

  /**
   * The module that originally provided this layout (if any).
   *
   * @var string
   */
  var $module;

  /**
   * The internal path of a layout.
   *
   * The layout path must match the provided menu item path. Set and get this
   * variable's value with Layout::getPath() and Layout::setPath().
   *
   * @var string
   */
  private $path;

  /**
   * The weight of this layout compared with other layouts at the same path.
   *
   * @var int
   */
  var $weight = 0;

  /**
   * The storage state of the layout.
   *
   * This represents whether this layout is has a default, user-created, or
   * overridden configuration. Possible values for this variable include the
   * constants LAYOUT_STORAGE_NORMAL, LAYOUT_STORAGE_OVERRIDE, or
   * LAYOUT_STORAGE_DEFAULT.
   *
   * @var int
   */
  var $storage;

  /**
   * Deprecated. Now replaced by $layout_template.
   *
   * @var NULL
   * @deprecated since 1.4.0
   */
  var $layout;

  /**
   * The name of the layout template used by this configuration.
   *
   * Although this property may be retrieved and set directly, if changing a
   * layout from one value to another, it is best to use Layout::setLayout(),
   * which will handle the moving of blocks from one region to another.
   *
   * @var string
   */
  var $layout_template;

  /**
   * Whether this layout is disabled.
   *
   * @var boolean
   */
  var $disabled = FALSE;

  /**
   * Whether this layout is locked (being currently edited).
   *
   * @var boolean
   */
  var $locked = FALSE;

  /**
   * An array of all conditional rules used by this layout.
   *
   * @var LayoutAccess[]
   */
  var $conditions = array();

  /**
   * An array of all relationships used by this layout.
   *
   * @var array
   */
  var $relationships = array();

  /**
   * A LayoutMenuItem containing menu properties for this layout.
   *
   * Layouts that are at user-defined paths (not overriding a module) must have
   * a menu item specified. Multiple layouts may share the same menu item.
   *
   * @var LayoutMenuItem
   */
  var $menu_item = NULL;

  /**
   * All user-specified settings for this layout.
   *
   * @var array
   */
  var $settings = array();

  /**
   * The main array of configured blocks, keyed by a generated ID per instance.
   *
   * @var Block[]
   */
  var $content = array();

  /**
   * Nested array that stores the arrangement of the configured blocks.
   *
   * This is keyed first by the region name and the position within that region,
   * and has the value of the block key within the $content array.
   *
   * @var array
   */
  var $positions = array();

  /**
   * If editing a layout, general storage of in-progress changes.
   *
   * @var array
   */
  var $in_progress;

  /**
   * The display renderer class used to handle this layout.
   *
   * @var string
   */
  var $renderer_name = 'standard';

  /**
   * An array of orphaned blocks when changing templates.
   *
   * @var array
   */
  var $orphaned_blocks = array();

  /**
   * The region where orphaned blocks are placed in new template.
   *
   * @var string
   */
  var $refuge_region;

  /**
   * An array of LayoutContext instances used by this menu item.
   *
   * @var LayoutContext[]
   */
  private $contexts = array();

  /**
   * An array of removed blocks, keyed by their UUID.
   *
   * This is being used in removeBlock() to keep track of blocks being removed
   * from the layout being edited. If the layout edits are saved, this is used
   * in layout_layout_presave() in order to give block modules a chance to react
   * on block removals. If the layout edits are canceled, there is no need to
   * clear this as that task is already being handled in
   * layout_settings_form_reset().
   *
   * @var array
   * @see removeBlock()
   * @see layout_layout_presave()
   */
  var $removed_blocks = array();

  /**
   * @var bool
   */
  public $is_new;

  /**
   * @var string
   */
  public $original_name;

  /**
   * Constructor for a Layout class.
   *
   * @param array $config
   *   An array of configuration data.
   */
  function __construct(array $config = array()) {
    foreach ($config as $property => $property_value) {
      $this->{$property} = $property_value;
    }

    // Specify default settings.
    $this->settings += array(
      'title' => '',
      'title_display' => LAYOUT_TITLE_DEFAULT,
      'title_block' => NULL,
    );

    // Adjust for the renaming of "layout" to "layout_template".
    // See https://github.com/backdrop/backdrop-issues/issues/998
    // @todo: Remove in 2.x.
    if (isset($this->layout) && !isset($this->layout_template)) {
      $this->layout_template = $this->layout;
      unset($this->layout);
    }

    // Set the storage type.
    if (isset($config['module'])) {
      if (empty($config['storage']) || $config['storage'] == LAYOUT_STORAGE_DEFAULT) {
        $this->storage = LAYOUT_STORAGE_DEFAULT;
      }
      else {
        $this->storage = LAYOUT_STORAGE_OVERRIDE;
      }
    }
    else {
      $this->storage = LAYOUT_STORAGE_NORMAL;
    }

    // Convert all stored contexts, conditions, and relationships to their
    // handler equivalents.
    $handlers = array(
      // Items are key => handler_type.
      'content' => 'block',
      'conditions' => 'layout_access',
      'contexts' => 'layout_context',
      'relationships' => 'layout_relationship',
    );
    foreach ($handlers as $property_key => $plugin_type) {
      foreach ($this->{$property_key} as $plugin_type_key => $plugin_data) {
        // Set the storage property for contexts stored in config.
        if ($property_key == 'contexts') {
          $plugin_data['data']['storage'] = TRUE;
        }
        $this->{$property_key}[$plugin_type_key] = layout_create_handler($plugin_type, $plugin_data['plugin'], $plugin_data['data']);
      }
    }

    // Ensure contexts is always set to an array.
    if (is_null($this->contexts)) {
      $this->contexts = array();
    }
  }

  /**
   * Save a layout to config.
   */
  function save() {
    if ($this->storage === LAYOUT_STORAGE_DEFAULT) {
      $this->storage = LAYOUT_STORAGE_OVERRIDE;
    }
    $is_new = !empty($this->is_new);

    // Don't save path based contexts as these are generated on the fly.
    foreach ($this->contexts as $key => $context) {
      if (!$context->storage) {
        unset($this->contexts[$key]);
      }
    }

    // Allow modules to act on layout data before saving.
    foreach (module_implements('layout_presave') as $module) {
      $function = $module . '_layout_presave';
      $function($this);
    }

    $data = array(
      'path' => $this->path,
      'name' => $this->name,
      'title' => $this->title,
      'description' => $this->description,
      'renderer_name' => $this->renderer_name,
      'module' => $this->module,
      'weight' => $this->weight,
      'storage' => $this->storage,
      'layout_template' => $this->layout_template,
      'disabled' => $this->disabled,
      'settings' => $this->settings,
      'positions' => $this->positions,
      'contexts' => $this->contexts,
      'relationships' => $this->relationships,
    );

    // Basic validation to prevent data corruption.
    if (empty($this->name)) {
      throw new LayoutSaveException(t('The layout must have a name specified to save.'));
    }

    // Convert all stored conditions to arrays.
    // @todo: Save per-layout contexts and relationships.
    $sub_parts = array(
      'content',
      'conditions',
      'contexts',
      'relationships',
    );
    foreach ($sub_parts as $config_type) {
      foreach ($this->$config_type as $config_type_key => $config_type_data) {
        unset($config_type_data->is_new);
        $data[$config_type][$config_type_key] = array(
          'plugin' => $config_type_data->plugin,
          'data' => $config_type_data->toArray(),
        );
      }
    }

    if (isset($this->original_name) && $this->original_name != $this->name) {
      config('layout.layout.' . $this->original_name)->delete();
    }
    config('layout.layout.' . $this->name)
      ->setData($data)
      ->save();

    // Parse each BlockText block for file IDs.
    $fids = array();
    foreach ($this->content as $content) {
      if (is_a($content, 'BlockText')) {
        $block_content = $content->settings['content'];
        $block_fids = filter_parse_file_fids($block_content);
        if (isset($block_fids)) {
          $fids = array_merge($fids, $block_fids);
        }
      }
    }
    $files = file_load_multiple($fids);
    foreach ($files as $fid => $file) {
      if ((int) ($file && $file->status) !== FILE_STATUS_PERMANENT) {
        // This makes the file "self-referencing", so it will never be deleted.
        // File usages are not removed for text blocks currently.
        // See https://github.com/backdrop/backdrop-issues/issues/2137.
        file_usage_add($file, 'file', 'file', $file->fid);
      }
    }

    // If this layout overrides an existing module path, reassign or delete
    // the layout menu item.
    if ((layout_provides_path($this->path) === FALSE) && menu_get_item($this->path)) {
      if ($this->menu_item) {
        $this->menu_item->delete();
      }
    }
    // If not overriding an existing module path, ensure that the layout always
    // has a layout menu item at the same path.
    else {
      if ($this->menu_item) {
        // If an existing menu item exists but this layout's path has changed,
        // reassign/delete the existing menu item, then create a new one.
        if ($this->menu_item->path !== $this->path) {
          $new_menu_item = clone($this->menu_item);
          $new_menu_item->path = $this->path;
          $new_menu_item->name = $this->name;
          $new_menu_item->is_new = TRUE;
          $this->menu_item->reassign();
          $this->menu_item = $new_menu_item;
          $this->menu_item->save();
        }
        else {
          if (isset($this->original_name) && $this->original_name !== $this->name && $this->menu_item->name === $this->original_name) {
            config('layout.menu_item.' . $this->original_name)->delete();
          }
          $this->menu_item->save();
        }
      }
    }
    layout_reset_caches();

    $new_this = layout_load($this->name);
    if ($is_new) {
      $new_this->invokeHook('insert');
    }
    else {
      $new_this->invokeHook('update');
    }
  }

  /**
   * Delete this layout.
   */
  function delete() {
    if ($this->storage === LAYOUT_STORAGE_NORMAL) {
      config('layout.layout.' . $this->name)->delete();
      // Reassign the menu item to a still-enabled layout (if any).
      if ($this->menu_item && $this->menu_item->name === $this->name) {
        $this->menu_item->reassign();
      }
      $this->invokeHook('delete');
      layout_reset_caches();
    }
    else {
      $this->disable();
    }
  }

  /**
   * Revert a layout to a module-provided default.
   */
  function revert() {
    if (!empty($this->module)) {
      // Restore the module-provided layout config.
      config('layout.layout.' . $this->name)->delete();
      config_install_default_config($this->module, 'layout.layout.' . $this->name);

      // Restore the module-provided menu settings if this layout provides the
      // active menu item.
      if ($this->menu_item && $this->menu_item->name === $this->name) {
        $this->menu_item->revert();
      }
      layout_reset_caches();
      $this->invokeHook('revert');
    }
  }

  /**
   * Disable a layout.
   */
  function disable() {
    $this->disabled = TRUE;
    $this->save();

    // Reassign the menu item to a still-enabled layout (if any).
    if ($this->menu_item && $this->menu_item->name === $this->name) {
      $this->menu_item->reassign();
    }
    $this->invokeHook('disable');
  }

  /**
   * Enable a layout.
   */
  function enable() {
    $this->disabled = FALSE;
    $this->save();

    // Check if the menu item needs to be assigned to this layout.
    if ($this->menu_item && $this->menu_item->name === $this->name) {
      $this->menu_item->reassign();
    }
    $this->invokeHook('enable');
  }

  /**
   * Invokes a hook on behalf of the layout.
   *
   * @param $hook
   *   One of 'insert', 'update', 'enable', 'disable', 'revert', or 'delete'.
   */
   protected function invokeHook($hook) {
    module_invoke_all('layout_' . $hook, $this);
  }

  /**
   * Clone a layout and return new Layout object.
   */
  function getClone() {
    $clone = clone($this);
    $clone->name = NULL;
    $clone->is_new = TRUE;
    $clone->module = NULL;
    $clone->storage = LAYOUT_STORAGE_NORMAL;

    // Copy over blocks with new UUIDs.
    $clone->content = array();
    $clone->positions = array();
    foreach ($this->positions as $region => $block_uuids) {
      foreach ($block_uuids as $block_uuid) {
        $new_block = $this->content[$block_uuid]->getClone();
        $clone->content[$new_block->uuid] = $new_block;
        $clone->positions[$region][] = $new_block->uuid;
      }
    }

    // Position the new layout one level above the current layout, assuming that
    // users will start with a generic layout, such as a global "node" layout,
    // then later create a more specific layout, such as "post nodes", which
    // should come before the more generic version.
    $clone->weight--;

    return $clone;
  }

  /**
   * Return a form for configuring this layout's settings.
   */
  function form(&$form, &$form_state) {
    // Most layouts do not have a form.
  }

  /**
   * Add a block to a particular region.
   *
   * @param string $block_module
   *   The module providing the block to be positioned.
   * @param string $block_delta
   *   The block delta from hook_block_info().
   * @param string $region_name
   *   The name of the region within the layout.
   * @param int $position
   *   The position of the block within the region. If not specified, the block
   *   will be the last block within the specified region.
   * @return Block
   *   The newly added block instance.
   */
  function addBlock($block_module, $block_delta, $region_name, $position = NULL) {
    $block = layout_create_handler('block', $block_module . ':' . $block_delta);
    $uuid = new Uuid();
    $block->uuid = $uuid->generate();

    $this->content[$block->uuid] = $block;
    $this->positions[$region_name][] = $block->uuid;

    // Move the block into the correct position.
    if (is_numeric($position)) {
      $positions = array();
      foreach ($this->positions[$region_name] as $n => $uuid) {
        if ($position == $n) {
          $positions[] = $block->uuid;
        }
        if ($uuid != $block->uuid) {
          $positions[] = $uuid;
        }
      }
      $this->positions[$region_name] = $positions;
    }

    return $block;
  }

  /**
   * Remove a block from a layout.
   *
   * @param string $block_uuid
   *   The UUID of the block being removed.
   */
  function removeBlock($block_uuid) {
    // Keep track of removed blocks so that block modules can react to their
    // removal (if layout edits are saved).
    $this->removed_blocks[$block_uuid] = $this->content[$block_uuid];

    unset($this->content[$block_uuid]);
    foreach ($this->positions as $region => $positions) {
      $key = array_search($block_uuid, $positions);
      if ($key !== FALSE) {
        unset($this->positions[$region][$key]);
        unset($this->content[$block_uuid]);
      }
    }

  }

  /**
   * Get a block's region position from a layout.
   *
   * @param string $block_uuid
   *   The UUID of the block whose position is being retrieved.
   * @param string $type
   *   Either "region" or "position". Defaults to "region".
   * @return string|int|FALSE
   *   Returns either the region name or the position of the block within that
   *   region. If the block is not found at all, returns FALSE.
   */
  function getBlockPosition($block_uuid, $type = 'region') {
    foreach ($this->positions as $region => $blocks) {
      foreach (array_values($blocks) as $position => $uuid) {
        if ($block_uuid === $uuid) {
          return $type === 'region' ? $region : $position;
        }
      }
    }
    return FALSE;
  }

  /**
   * Move an existing block from its current region to a new one.
   *
   * @param string $block_uuid
   *   The UUID of the block whose position is being retrieved.
   * @param string $region_name
   *   The name of the region to which the block will be moved.
   */
  function setBlockPosition($block_uuid, $region_name, $position = NULL) {
    // Set to the last position if position is greater than available places.
    if (empty($this->positions[$region_name]) || $position > count($this->positions[$region_name])) {
      $position = NULL;
    }

    // Find the current region.
    if ($current_region = $this->getBlockPosition($block_uuid)) {
      $current_position = array_search($block_uuid, $this->positions[$current_region]);
      // Remove the block from the current region.
      if ($current_position !== FALSE) {
        unset($this->positions[$current_region][$current_position]);
        $this->positions[$current_region] = array_values($this->positions[$current_region]);
      }
    }

    // Add the block to the new region in the correct position.
    if (isset($region_position)) {
      $new_positions = array();
      foreach (array_values($this->positions[$region_name]) as $region_position => $existing_uuid) {
        if ($region_position === $position) {
          $new_positions[] = $block_uuid;
        }
        $new_positions[] = $existing_uuid;
      }
      $this->positions[$region_name] = $new_positions;
    }
    else {
      $this->positions[$region_name][] = $block_uuid;
    }
  }

  /**
   * Validate the settings form.
   */
  function formValidate($form, &$form_state) {
    // Most layouts do not have a form.
  }

  /**
   * Save the settings added in the form method.
   */
  function formSubmit($form, &$form_state) {
    // Most layouts do not have a form.
  }

  /**
   * Set a layout path.
   */
  function setPath($path) {
    if (empty($this->menu_item) || $this->menu_item->path !== $path) {
      // Use an existing menu item if one exists at this path.
      if ($existing_item = layout_menu_item_load_multiple_by_path($path)) {
        $this->menu_item = $existing_item;
      }
      // If this layout doesn't have a menu item, make one. If the path is
      // provided by another module, no menu item is necessary.
      elseif (layout_provides_path($path) === NULL) {
        if (empty($this->menu_item)) {
          $menu_item_settings = array(
            'path' => $path,
            'name' => $this->name,
          );
          $this->menu_item = new LayoutMenuItem($menu_item_settings);
        }
        else {
          $this->menu_item->path = $path;
        }
      }
      else {
        if ($this->menu_item) {
          $this->menu_item->delete();
        }
        $this->menu_item = NULL;
      }
    }
    // On any path change, reset contexts.
    if ($this->path !== $path) {
      $this->path = $path;
      $this->resetContexts();
    }
  }

  /**
   * Return a layout's path based on its assigned menu item.
   *
   * @return string
   */
  function getPath() {
    if ($this->menu_item) {
      return $this->menu_item->path;
    }
    else {
      return $this->path;
    }
  }

  /**
   * Set the layout template, accommodating moving blocks if necessary.
   *
   * @param $layout_name
   *   The new layout name.
   * @param $region_mapping
   *   An array of region names, each keyed by the old region name and the value
   *   of the new region name. If empty, all known regions will be copied from
   *   one to the other. To discard the contents of a region, set the value
   *   to FALSE.
   */
  function setLayoutTemplate($layout_name, $region_mapping = array()) {
    // If not changing anything, return as a no-op.
    if ($layout_name === $this->layout_template && empty($region_mapping)) {
      return;
    }
    // If no layout has yet been set, then we have no copying to do.
    if (!isset($this->layout_template)) {
      $this->layout_template = $layout_name;
      return;
    }

    $old_layout_template_info = layout_get_layout_template_info($this->layout_template);
    $new_layout_template_info = layout_get_layout_template_info($layout_name);
    // Set up the array with region names for the new layout.
    $new_positions = array();
    $new_regions = array_keys($new_layout_template_info['regions']);
    foreach ($new_regions as $new_region_name) {
      $new_positions[$new_region_name] = array();
    }

    // Do the mapping in the order provided (if any).
    if ($region_mapping) {
      $regions = array_keys($region_mapping);
      $regions = array_unique(array_merge($regions, array_keys($old_layout_template_info['regions'])));
    }
    else {
      $regions = array_keys($old_layout_template_info['regions']);
    }
    foreach ($regions as $old_region_name) {
      if (isset($region_mapping[$old_region_name])) {
        $new_region_name = $region_mapping[$old_region_name];
      }
      elseif (array_key_exists($old_region_name, $new_layout_template_info['regions'])) {
        $new_region_name = $old_region_name;
      }
      else {
        $new_region_name = '';
      }

      if (isset($this->positions[$old_region_name])) {
        foreach ($this->positions[$old_region_name] as $uuid) {
          if (!empty($new_region_name)) {
            $new_positions[$new_region_name][] = $uuid;
          }
          elseif ($new_region_name === '') {
            $this->orphaned_blocks[] = $uuid;
          }
          else {
            unset($this->content[$uuid]);
          }
        }
      }
    }

    // Assign orphan blocks to the default or last region, if the former doesn't
    // exist.
    if (!empty($this->orphaned_blocks)) {
      end($new_positions);
      $this->refuge_region = !empty($new_layout_template_info['default region']) ? $new_layout_template_info['default region'] : key($new_positions);
      foreach ($this->orphaned_blocks as $uuid) {
        $new_positions[$this->refuge_region][] = $uuid;
      }
    }

    $this->layout_template = $layout_name;
    $this->positions = $new_positions;
    module_invoke_all('layout_template_change', $this, $old_layout_template_info['name']);
  }

  /**
   * Return all contexts (from both the layout and menu item) for this Layout.
   *
   * @var int $include_types
   *   Include a specific list of contexts based on how they are used.
   *
   *   This is loaded from the list of constants provided in the
   *   LayoutContext class, which includes the following:
   *
   *   - USAGE_TYPE_ALL - All contexts from all possible sources.
   *   - USAGE_TYPE_CUSTOM - Contexts manually specified in configuration.
   *   - USAGE_TYPE_MENU - Contexts automatically provided from a menu path.
   *   - USAGE_TYPE_SYSTEM - Global contexts such as logged-in user.
   *   - USAGE_TYPE_RELATIONSHIP - Contexts added through relationships.
   *
   * @return LayoutContext[]
   */
  function getContexts($include_types = LayoutContext::USAGE_TYPE_ALL) {
    $contexts_cache = &backdrop_static(__METHOD__, array());
    if (isset($contexts_cache[$this->name])) {
      return $this->getContextsByType($contexts_cache[$this->name], $include_types);
    }

    // Initialize a list of contexts if not defined.
    if (is_null($this->contexts)) {
      $this->contexts = array();
    }

    // Load objects for stored contexts.
    foreach ($this->contexts as $key => $context) {
      if (!isset($context->position) && !is_object($context->data)) {
        $context_info = layout_get_context_info($context->plugin);
        if (isset($context_info['load callback'])) {
          $load_arguments = array_values($context->settings);
          $context_data = call_user_func_array($context_info['load callback'], $load_arguments);
          $context->setData($context_data);
        }
      }
    }

    // Load contexts from paths or menu items if available.
    if ($this->menu_item || $this->path) {
      if ($this->menu_item) {
        $this->contexts += $this->menu_item->getContexts();
      }
      else {
        $this->contexts += layout_context_required_by_path($this->path);
      }
    }

    // Load contexts from relationships.
    if ($include_types & LayoutContext::USAGE_TYPE_RELATIONSHIP) {
      $this->contexts += $this->getContextsFromRelationships();
    }

    // Add on the current user context, which is always available.
    if (!isset($this->contexts['current_user'])) {
      $this->contexts['current_user'] = layout_current_user_context();
    }

    // Add on the overrides path context, which is always available.
    if (empty($this->menu_item) && !isset($this->contexts['overrides_path'])) {
      $this->contexts['overrides_path'] = layout_create_context('overrides_path', array(
        'name' => 'overrides_path',
        'locked' => TRUE,
      ));
    }

    $contexts_cache[$this->name] = $this->contexts;

    return $this->getContextsByType($this->contexts, $include_types);
  }

  /**
   * Return an array of contexts filtered by types specified.
   *
   * @param array $contexts
   *   The array of contexts to be filtered.
   * @var int $include_types
   *   Include a specific list of contexts based on how they are used.
   * @return LayoutContext[]
   */
  private function getContextsByType($contexts, $include_types = LayoutContext::USAGE_TYPE_ALL) {
    // Return all contexts if requested.
    if ($include_types === LayoutContext::USAGE_TYPE_ALL) {
      return $contexts;
    }

    // Otherwise filter down the list of contexts to only those requested.
    $return_contexts = array();
    foreach ($contexts as $key => $context) {
      if ($context->usageType & $include_types) {
        $return_contexts[$key] = $context;
      }
    }

    return $return_contexts;
  }

  /**
   * Set the internally stored contexts.
   */
  function setContexts($key, $context) {
    $this->contexts[$key] = $context;
  }

  /**
   * Clear a stored context.
   */
  function clearContexts($key) {
    unset($this->contexts[$key]);
    backdrop_static_reset('Layout::getContexts');
  }

  /**
   * Reset the internally stored contexts.
   *
   * This is used before storing a layout, or when fresh contexts are important,
   * such as when actively changing the layout's path.
   */
  function resetContexts() {
    foreach ($this->contexts as $key => $context) {
      if (!$context->storage) {
        unset($this->contexts[$key]);
      }
    }
    if ($this->menu_item) {
      $this->menu_item->resetContexts();
    }
  }

  /**
   * Check if the layout has a context of a particular name.
   *
   * @param array $required_contexts
   *   An unindexed array of context plugin names.
   * @return boolean
   *   TRUE if this layout has all the required contexts, FALSE otherwise.
   */
  function hasContexts($required_contexts) {
    $all_available_contexts = $this->getContexts();
    $context_counts = array_count_values($required_contexts);
    foreach (array_unique($required_contexts) as $required_context_name) {
      foreach ($all_available_contexts as $available_context) {
        if ($available_context->isA($required_context_name)) {
          // Matching context available, continue to the next one.
          $context_counts[$required_context_name] = ($context_counts[$required_context_name] > 0) ? $context_counts[$required_context_name] - 1 : 0;
        }
      }
    }
    
    return empty(array_filter($context_counts));
  }

  /**
   * @param LayoutRelationship $relationship
   *   A relationship object.
   *
   * @param LayoutContext $source_context
   *   The context this relationship is based upon.
   *
   * @return LayoutContext|FALSE
   *   A context object if one can be loaded.
   */
  private function getContextFromRelationship(LayoutRelationship $relationship, LayoutContext $source_context) {
    $new_context = clone($source_context);
    $context = $relationship->getContext($new_context);
    if ($context) {
      $context->usageType = LayoutContext::USAGE_TYPE_RELATIONSHIP;
      return $context;
    }
    return FALSE;
  }

  /**
   * Load the contexts based on this Layout's relationship configuration.
   */
  private function getContextsFromRelationships() {
    $contexts = array();
    $contexts_from_relationships_list = array();
    foreach ($this->relationships as $relationship_name => $relationship_data) {
      $key = 'relationship_' . $relationship_name;
      if (!isset($contexts_from_relationships_list[$key])) {
        foreach ($this->contexts as $context) {
          if ($context->plugin == $relationship_data->context) {
            $plugin_array = explode(':', $relationship_data->settings['relationship']);
            if ($plugin_array[1] != 'child') {
              $relationship_data->childDelta = $plugin_array[1];
            }
            $contexts_from_relationships_list[$key] = $this->getContextFromRelationship($relationship_data, $context);
          }
        }
      }
    }
    if ($contexts_from_relationships_list) {
      foreach ($contexts_from_relationships_list as $key => $context_from_relationships) {
        if (is_object($context_from_relationships)) {
          $contexts[$key] = $context_from_relationships;
        }
      }
    }
    return $contexts;
  }

  /**
   * Check if this layout is a fallback default layout.
   */
  function isDefault() {
    return in_array($this->name, array('default', 'admin_default'));
  }

  /**
   * Check access to this layout based on the current contexts.
   *
   * This method is generally called on all layouts that share the same path.
   * The first layout that grants access will be used to render the page, and if
   * no layouts grant access, then Layout module will fall through to using the
   * default site-wide layout.
   */
  function checkAccess() {
    $contexts = $this->getContexts();
    foreach ($this->conditions as $condition) {
      $condition->setContexts($contexts);
      if (!$condition->checkAccess()) {
        return FALSE;
      }
    }
    return TRUE;
  }
}
