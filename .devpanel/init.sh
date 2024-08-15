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

#== If webRoot has not been difined, we will set appRoot to webRoot
if [[ ! -n "$WEB_ROOT" ]]; then
  export WEB_ROOT=$APP_ROOT
fi
STATIC_FILES_PATH="$WEB_ROOT/files"
SETTINGS_FILES_PATH="$WEB_ROOT/settings.php"


#== Init Backdrop
if [[ ! -d "$APP_ROOT/core" ]]; then
  echo "Initial backdrop ..."
  cd /tmp && curl -sLo backdrop.tar.gz https://github.com/backdrop/backdrop/tarball/1.28.2
  tar xzf backdrop.tar.gz -C $APP_ROOT --strip-components=1
fi

sudo mkdir -p $STATIC_FILES_PATH
sudo mkdir -p $STATIC_FILES_PATH/config_$DP_APP_ID/active
sudo mkdir -p $STATIC_FILES_PATH/config_$DP_APP_ID/staging
sudo chmod 775 -R $STATIC_FILES_PATH


#== Create settings files

echo "Create settings file ..."
sudo cp $APP_ROOT/.devpanel/backdrop-cms-settings.php $SETTINGS_FILES_PATH


#== Generate hash salt
echo 'Generate hash salt ...'
DRUPAL_HASH_SALT=$(openssl rand -hex 32);
sudo sed -i -e "s/^\$settings\['hash_salt'\].*/\$settings\['hash_salt'\] = '$DRUPAL_HASH_SALT';/g" $SETTINGS_FILES_PATH


#== Drush Site Install
if [[ $(mysql -h$DB_HOST -P$DB_PORT -u$DB_USER -p$DB_PASSWORD $DB_NAME -e "show tables;") == '' ]]; then
  echo "Site installing ..."
  cd $APP_ROOT
  #Allow drush to overide files
  sudo chown -R www:www $STATIC_FILES_PATH
  drush si --account-name=devpanel --account-pass=devpanel --db-url="mysql://$DB_USER:$DB_PASSWORD@$DB_HOST:$DB_PORT/$DB_NAME" -y
  drush user-password devpanel --password="devpanel"
  drush cc all
fi

echo "Update permission ..."
sudo chown -R www-data:www-data $STATIC_FILES_PATH
sudo chown www:www $SETTINGS_FILES_PATH
sudo chmod 644 $SETTINGS_FILES_PATH

#== Revert file being override
git checkout .gitignore
git checkout README.md
