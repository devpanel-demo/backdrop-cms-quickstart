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
SETTINGS_FILES_PATH="$WEB_ROOT/settings.local.php"


#== Init Backdrop

# Using tar.gz download from source shows version as 1.x in .info files
#if [[ ! -d "$APP_ROOT/core" ]]; then
#  echo "Initial backdrop ..."
#  cd /tmp && curl -sLo backdrop.tar.gz https://github.com/backdrop/backdrop/tarball/1.28.2
#  tar xzf backdrop.tar.gz -C $APP_ROOT --strip-components=1
#fi

# Using zip download of release shows version and date in .info files
if [[ ! -d "$APP_ROOT/core" ]]; then
  echo "Initial backdrop ..."
  cd /tmp && wget https://github.com/backdrop/backdrop/releases/download/1.28.2/backdrop.zip
  cd $APP_ROOT
  unzip /tmp/backdrop.zip && rm -f backdrop.zip && mv -f ./backdrop/{.,}* . ; rm -rf backdrop
fi

sudo mkdir -p $STATIC_FILES_PATH
sudo mkdir -p $STATIC_FILES_PATH/config_$DP_APP_ID/active
sudo mkdir -p $STATIC_FILES_PATH/config_$DP_APP_ID/staging
# Create private files directory
sudo mkdir -p $STATIC_FILES_PATH/private
sudo chmod 775 -R $STATIC_FILES_PATH


#== Create settings files

echo "Create settings file ..."
sudo cp $APP_ROOT/.devpanel/backdrop-cms-settings.local.php $SETTINGS_FILES_PATH


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
  mysql -h$DB_HOST -P$DB_PORT -u$DB_USER -p$DB_PASSWORD $DB_NAME -e "UPDATE users SET name = 'devpanel' WHERE uid = 1;"
  drush user-password devpanel --password="devpanel"
  drush cc all
fi

echo "Update permission ..."
sudo chown -R www-data:www-data $STATIC_FILES_PATH
sudo chown www:www $SETTINGS_FILES_PATH
sudo chmod 644 $SETTINGS_FILES_PATH

#== Change permissions for UI downloads
cd $APP_ROOT
# Change ownership
sudo chown -R www:www-data *
# Change all directory permissions to 775 "rwxrwxr-x"
sudo find * -type d -exec chmod ug=rwx,o=rx '{}' \;
# Change all file permissions to 664 "rw-rw-r--"
sudo find * -type f -exec chmod ug=rw,o=r '{}' \;
# Change all .txt except robots .txt to 600 "rw-------"
sudo find * -type f -iname "*.txt" -exec chmod u=rw,go= '{}' \;
sudo chmod u=rw,go=r 'robots.txt' ;
# Change all .md file permissions to 600 "rw-------"
sudo find * -type f -iname "README.md" -exec chmod u=rw,go= '{}' \;
# Change all .patch and *.orig files to 600 "rw-------"
sudo find * -type f -iname "*.patch" -exec chmod u=rw,go= '{}' \;
sudo find * -type f -iname "*.orig" -exec chmod u=rw,go= '{}' \;

#== Revert file being override
git checkout .gitignore
git checkout README.md
