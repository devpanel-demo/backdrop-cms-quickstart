<?php
/**
 * @file
 * Theme functions for the Filter module.
 */

/**
 * Returns HTML for a text format-enabled form element.
 *
 * @param $variables
 *   An associative array containing:
 *   - element: A render element containing #children and #description.
 *
 * @ingroup themeable
 */
function theme_text_format_wrapper($variables) {
  $element = $variables['element'];
  $output = '<div class="text-format-wrapper">';
  $output .= $element['#children'];
  if (!empty($element['#description']) && $element['#description_display'] == 'after') {
    $output .= theme('form_element_description', $variables);
  }
  $output .= "</div>\n";

  return $output;
}

/**
 * Returns HTML for a captioned item, usually an image.
 */
function theme_filter_caption($variables) {
  $output = '';
  $output .= '<figure' . backdrop_attributes($variables['attributes']) . '>';
  $output .= $variables['item'];
  $output .= '<figcaption>' . $variables['caption'] . '</figcaption>';
  $output .= '</figure>';
  return $output;
}

/**
 * Returns HTML for a link to the more extensive filter tips.
 *
 * @ingroup themeable
 */
function theme_filter_tips_more_info() {
  return '<p>' . l(t('More information about text editors'), 'filter/tips', array('attributes' => array('target' => '_blank'))) . '</p>';
}

/**
 * Returns HTML for guidelines for a text format.
 *
 * @param $variables
 *   An associative array containing:
 *   - format: An object representing a text format.
 *
 * @ingroup themeable
 */
function theme_filter_guidelines($variables) {
  $format = $variables['format'];
  $attributes['class'][] = 'filter-guidelines-item';
  $attributes['class'][] = 'filter-guidelines-' . $format->format;
  $output = '<div' . backdrop_attributes($attributes) . '>';
  $output .= '<h3>' . check_plain($format->name) . '</h3>';
  $output .= theme('filter_tips', array('tips' => _filter_tips($format->format, FALSE)));
  $output .= '</div>';
  return $output;
}

/**
 * Returns HTML for the text format administration overview form.
 *
 * @param $variables
 *   An associative array containing:
 *   - form: A render element representing the form.
 *
 * @ingroup themeable
 */
function theme_filter_admin_overview($variables) {
  $form = $variables['form'];

  $rows = array();
  $first_disabled_row = TRUE;
  foreach (element_children($form['formats']) as $id) {
    $form['formats'][$id]['weight']['#attributes']['class'] = array('text-format-order-weight');
    $row = array(
      'data' => array(
        backdrop_render($form['formats'][$id]['name']),
        backdrop_render($form['formats'][$id]['editor']),
        backdrop_render($form['formats'][$id]['roles']),
        backdrop_render($form['formats'][$id]['weight']),
        backdrop_render($form['formats'][$id]['operations']),
      ),
    );
    if ($form['formats'][$id]['#status']) {
      $row['class'] = array('draggable', 'enabled');
    }
    else {
      // Add a heading above the first disabled row.
      if ($first_disabled_row) {
        $first_disabled_row = FALSE;
        // Create a row that spans all columns.
        $rows[] = array(
          'data' => array(
            array(
              'data' => t('Disabled'),
              'colspan' => 5,
              'header' => TRUE,
            ),
          ),
        );
      }
      $row['class'] = array('disabled');
    }
    $rows[] = $row;
  }
  $header = array(t('Name'), t('Editor'), t('Roles'), t('Weight'), t('Operations'));
  $output = backdrop_render($form['help']);
  $output .= theme('table', array('header' => $header, 'rows' => $rows, 'attributes' => array('id' => 'text-format-order')));
  $output .= backdrop_render_children($form);

  backdrop_add_tabledrag('text-format-order', 'order', 'sibling', 'text-format-order-weight');

  return $output;
}

/**
 * Returns HTML for a text format's filter order form.
 *
 * @param $variables
 *   An associative array containing:
 *   - element: A render element representing the form.
 *
 * @ingroup themeable
 */
function theme_filter_admin_format_filter_order($variables) {
  $element = $variables['element'];

  // Filter order (tabledrag).
  $rows = array();
  foreach (element_children($element, TRUE) as $name) {
    $element[$name]['weight']['#attributes']['class'][] = 'filter-order-weight';
    $row_class =  $element[$name]['status']['#default_value'] ? 'draggable' : 'draggable disabled-row';

    $rows[] = array(
      'data' => array(
        backdrop_render($element[$name]['filter']),
        backdrop_render($element[$name]['status']),
        backdrop_render($element[$name]['configure_button']),
        backdrop_render($element[$name]['weight']),
      ),
      'class' => array($row_class),
    );
  }
  $output = backdrop_render_children($element);
  $header = array(t('Filter'), t('Enabled'), t('Configure'), '');
  $output .= theme('table', array('header' => $header, 'rows' => $rows, 'attributes' => array('id' => 'filter-order')));
  backdrop_add_tabledrag('filter-order', 'order', 'sibling', 'filter-order-weight', NULL, NULL, TRUE);

  return $output;
}

/**
 * Returns HTML for a set of filter tips.
 *
 * @param $variables
 *   An associative array containing:
 *   - tips: An array containing descriptions and a CSS ID in the form of
 *     'module-name/filter-id' (only used when $long is TRUE) for each
 *     filter in one or more text formats. Example:
 *     @code
 *       array(
 *         'Raw HTML' => array(
 *           0 => array(
 *             'tip' => 'Web page addresses and email addresses turn into links automatically.',
 *             'id' => 'filter/2',
 *           ),
 *         ),
 *       );
 *     @endcode
 *   - long: (optional) Whether the passed-in filter tips contain extended
 *     explanations, i.e. intended to be output on the path 'filter/tips'
 *     (TRUE), or are in a short format, i.e. suitable to be displayed below a
 *     form element. Defaults to FALSE.
 *
 * @see _filter_tips()
 * @ingroup themeable
 */
function theme_filter_tips($variables) {
  $tips = $variables['tips'];
  $long = $variables['long'];
  $output = '';

  $multiple = count($tips) > 1;
  if ($multiple) {
    $output = '<h2>' . t('Text formats') . '</h2>';
  }

  if (count($tips)) {
    if ($multiple) {
      $output .= '<div class="compose-tips">';
    }
    foreach ($tips as $name => $tip_list) {
      if ($multiple) {
        $output .= '<div class="filter-type filter-' . backdrop_html_class($name) . '">';
        $output .= '<h3>' . check_plain($name) . '</h3>';
      }

      if (count($tip_list) > 0) {
        $output .= '<ul class="tips">';
        foreach ($tip_list as $tip) {
          $output .= '<li' . ($long ? ' id="filter-' . str_replace("/", "-", $tip['id']) . '">' : '>') . $tip['tip'] . '</li>';
        }
        $output .= '</ul>';
      }

      if ($multiple) {
        $output .= '</div>';
      }
    }
    if ($multiple) {
      $output .= '</div>';
    }
  }

  return $output;
}

function theme_filter_format_editor_image_form($variables) {
  $form = $variables['form'];

  $actions = backdrop_render($form['actions']);

  $output = '';
  $output .= '<div class="image-form-wrapper">';

  $output .= '<div class="editor-image-fields">';
  $output .= backdrop_render_children($form);
  $output .= '</div>';

  $output .= '</div>';

  $output .= $actions;
  return $output;
}
