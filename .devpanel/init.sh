#!/bin/bash
# ---------------------------------------------------------------------
# Copyright (C) 2021 DevPanel
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU Affero General Public License as
# published by the Free Software Foundation version 3 of the
# License.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU Affero General Public License for more details.
#
# For GNU Affero General Public License see <https://www.gnu.org/licenses/>.
# ----------------------------------------------------------------------

#== If webRoot has not been defined, we will set appRoot to webRoot
if [[ ! -n "$WEB_ROOT" ]]; then
  export WEB_ROOT=$APP_ROOT
fi
STATIC_FILES_PATH="$WEB_ROOT/files"
SETTINGS_FILES_PATH="$WEB_ROOT/settings.local.php"


#== Init Backdrop
if [[ ! -d "$APP_ROOT/core" ]]; then
  echo "Initial backdrop ..."
  cd /tmp && curl -sLo backdrop.tar.gz https://github.com/backdrop/backdrop/tarball/1.24.1
  tar xzf backdrop.tar.gz -C $APP_ROOT --strip-components=1
fi


# #Securing file permissions and ownership
# #https://www.drupal.org/docs/security-in-drupal/securing-file-permissions-and-ownership
[[ ! -d $STATIC_FILES_PATH ]] && sudo mkdir --mode 775 $STATIC_FILES_PATH || sudo chmod 775 -R $STATIC_FILES_PATH
sudo chown -R www:www $STATIC_FILES_PATH

#== Create settings files
if [[ ! -f "$SETTINGS_FILES_PATH" ]]; then
  echo "Create settings file ..."
  sudo cp $APP_ROOT/.devpanel/backdrop-cms-settings.local.php $SETTINGS_FILES_PATH
fi

#== Generate hash salt
echo 'Generate hash salt ...'
DRUPAL_HASH_SALT=$(openssl rand -hex 32);
sudo sed -i -e "s/^\$settings\['hash_salt'\].*/\$settings\['hash_salt'\] = '$DRUPAL_HASH_SALT';/g" $SETTINGS_FILES_PATH


#== Drush Site Install
if [[ $(drush status bootstrap) == '' ]]; then
  echo "Site installing ..."
  cd $APP_ROOT
  drush si --account-name=devpanel --account-pass=devpanel --db-url="mysql://$DB_USER:$DB_PASSWORD@$DB_HOST:$DB_PORT/$DB_NAME" -y
  drush cc all
fi

#== Update permission
echo "Update permision ..."
sudo chown -R www:www $APP_ROOT
sudo chown -R www:www $STATIC_FILES_PATH
