<?php
/**
 * @file
 * Local Backdrop CMS configuration file.
 */

/**
 * Database configuration:
 *
 * Most sites can configure their database by entering the connection string
 * below. If using master/slave databases or multiple connections, see the
 * advanced database documentation at
 * https://api.backdropcms.org/database-configuration
 */

$databases['default']['default'] = array(
  'driver' => getenv('DB_DRIVER'),
  'database' =>  getenv('DB_NAME'),
  'username' => getenv('DB_USER') ,
  'password' => getenv('DB_PASSWORD'),
  'host' => getenv('DB_HOST'),
  'port' => getenv('DB_PORT')
);

$database_prefix = '';

/**
 * Configuration storage
 *
 * By default configuration will be stored in the filesystem, using the
 * directories specified in the $config_directories setting. Optionally,
 * configuration can be store in the database instead of the filesystem.
 * Switching this option on a live site is not currently supported without some
 * manual work.
 *
 * Example using the database for live and file storage for staging:
 * @code
 * $settings['config_active_class'] = 'ConfigDatabaseStorage';
 * $settings['config_staging_class'] = 'ConfigFileStorage';
 * @endcode
 */
// $settings['config_active_class'] = 'ConfigFileStorage';
// $settings['config_staging_class'] = 'ConfigFileStorage';

/**
 * Site configuration files location.
 *
 * By default these directories are stored within the files directory with a
 * hashed path. For the best security, these directories should be in a location
 * that is not publicly accessible through a web browser.
 *
 * Example using directories one parent level up:
 * @code
 * $config_directories['active'] = '../config/active';
 * $config_directories['staging'] = '../config/staging';
 * @endcode
 *
 * Example using absolute paths:
 * @code
 * $config_directories['active'] = '/home/myusername/config/active';
 * $config_directories['staging'] = '/home/myusername/config/staging';
 * @endcode
 */
$config_directories['active'] = 'files/config_' . getenv('DP_APP_ID') . '/active';
$config_directories['staging'] = 'files/config_' . getenv('DP_APP_ID') . '/staging';

/**
 * Skip the configuration staging directory cleanup
 *
 * When the configuration files are in version control, it may be preferable to
 * not empty the staging directory after each sync.
 */
// $config['system.core']['config_sync_clear_staging'] = 0;

/**
 * Access control for update.php script.
 *
 * If you are updating your Backdrop installation using the update.php script
 * but are not logged in using either an account with the "Administer software
 * updates" permission or the site maintenance account (the account that was
 * created during installation), you will need to modify the access check
 * statement below. Change the FALSE to a TRUE to disable the access check.
 * After finishing the upgrade, be sure to open this file again and change the
 * TRUE back to a FALSE!
 */
$settings['update_free_access'] = FALSE;

/**
 * Salt for one-time login links and cancel links, form tokens, etc.
 *
 * This variable will be set to a random value by the installer. All one-time
 * login links will be invalidated if the value is changed. Note that if your
 * site is deployed on a cluster of web servers, you must ensure that this
 * variable has the same value on each server. If this variable is empty, a hash
 * of the serialized database credentials will be used as a fallback salt.
 *
 * For enhanced security, you may set this variable to a value using the
 * contents of a file outside your docroot that is never saved together
 * with any backups of your Backdrop files and database.
 *
 * Example:
 * @code
 * $settings['hash_salt'] = file_get_contents('/home/example/salt.txt');
 * @endcode
 *
 */
$settings['hash_salt'] = '';

/**
 * Trusted host configuration (optional but highly recommended).
 *
 * Since the HTTP Host header can be set by the user making the request, it
 * is possible for malicious users to override it and create an attack vector.
 * To protect against these sort of attacks, Backdrop supports checking a list
 * of trusted hosts.
 *
 * To enable the trusted host protection, specify the allowable hosts below.
 * This should be an array of regular expression patterns representing the hosts
 * you would like to allow.
 *
 * For example, this will allow the site to only run from www.example.com:
 * @code
 * $settings['trusted_host_patterns'] = array(
 *   '^www\.example\.com$',
 * );
 * @endcode
 *
 * If you are running a site on multiple domain names, you should specify all of
 * the host patterns that are allowed by your site. For example, this will allow
 * the site to run off of all variants of example.com and example.org, with all
 * subdomains included:
 * @code
 * $settings['trusted_host_patterns'] = array(
 *   '^example\.com$',
 *   '^.+\.example\.com$',
 *   '^example\.org',
 *   '^.+\.example\.org',
 * );
 * @endcode
 *
 * If you do not need this functionality (such as in development environments or
 * if protection is at another layer), you can suppress the status report
 * warning by setting this value to FALSE:
 * @code
 * $settings['trusted_host_patterns'] = FALSE;
 * @endcode
 *
 * For more information about trusted host patterns, see the documentation at
 * https://api.backdropcms.org/documentation/trusted-host-settings
 *
 * @see backdrop_valid_http_host()
 * @see backdrop_check_trusted_hosts()
 * @see system_requirements()
 */
// $settings['trusted_host_patterns'] = array('^www\.example\.com$');

/**
 * Base URL (optional).
 *
 * If Backdrop is generating incorrect URLs on your site, which could be in HTML
 * headers (links to CSS and JS files) or visible links on pages (such as in
 * menus), uncomment the Base URL statement below and fill in the absolute URL
 * to your Backdrop installation.
 *
 * You might also want to force users to use a given domain.
 * See the .htaccess file for more information.
 *
 * Examples:
 *   $base_url = 'http://www.example.com';
 *   $base_url = 'http://www.example.com:8888';
 *   $base_url = 'http://www.example.com/backdrop';
 *   $base_url = 'https://www.example.com:8888/backdrop';
 *
 * It is not allowed to have a trailing slash; Backdrop will add it for you.
 */
// $base_url = 'http://www.example.com'; // NO trailing slash!

/**
 * Drupal backwards compatibility.
 *
 * By default, Backdrop 1.0 includes a compatibility layer to keep it compatible
 * with Drupal 7 APIs. Backdrop core itself does not use this compatibility
 * layer however. You may disable it if all the modules you're running were
 * built for Backdrop.
 */
$settings['backdrop_drupal_compatibility'] = TRUE;