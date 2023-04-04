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
DEFAULT_SETTINGS_FILE="https://raw.githubusercontent.com/backdrop/backdrop/1.24.1/settings.php"
CONFIG_DIR="";

#== Create settings files
echo 'Create database settings ...';
if [[ ! -f "$SETTINGS_FILES_PATH" ]]; then
  curl -o $SETTINGS_FILES_PATH $DEFAULT_SETTINGS_FILE --silent
fi
sed -i -e "s/^\$database =.*/\$database = 'mysql:\/\/'.getenv\('DB_USER'\).':'.getenv\('DB_PASSWORD'\).'@'.getenv\('DB_HOST'\).'\/'.getenv\('DB_NAME'\);/g" $SETTINGS_FILES_PATH



#== Extract static files
if [[ $(drush status bootstrap) == '' ]]; then
  if [[ -f "$APP_ROOT/.devpanel/dumps/files.tgz" ]]; then
    echo  'Extract static files ...'
    mkdir -p $STATIC_FILES_PATH
    sudo tar xzf "$APP_ROOT/.devpanel/dumps/files.tgz" -C $STATIC_FILES_PATH
  fi

  #== Import mysql files
  if [[ -f "$APP_ROOT/.devpanel/dumps/db.sql.tgz" ]]; then
    echo  'Extract mysql files ...'
    SQLFILE=$(tar tzf $APP_ROOT/.devpanel/dumps/db.sql.tgz)
    tar xzf "$APP_ROOT/.devpanel/dumps/db.sql.tgz" -C /tmp/
    mysql -h$DB_HOST -u$DB_USER -p$DB_PASSWORD $DB_NAME < /tmp/$SQLFILE
    rm /tmp/$SQLFILE
  fi
fi


#== Point config directory
if [[ -d $(find $APP_ROOT -maxdepth 1 -type d -name  'config_*' |  head -n 1) ]];  then
  CONFIG_DIR=$(find $APP_ROOT -maxdepth 1 -type d -name  'config_*'| head -n 1 | sed 's/\//\\\//g')
fi
if [[ -d $(find $STATIC_FILES_PATH -maxdepth 1 -type d -name  'config_*' |  head -n 1) ]];  then
  CONFIG_DIR=$(find $STATIC_FILES_PATH -maxdepth 1 -type d -name  'config_*'| head -n 1 | sed 's/\//\\\//g')
fi
if [[ $CONFIG_DIR != '' ]]; then
  echo 'Create config settings ...';
  sed -i -e "s/^\$config_directories\['active'\].*/\$config_directories\['active'\] = '$CONFIG_DIR\/active';/g" $SETTINGS_FILES_PATH
  sed -i -e "s/^\$config_directories\['staging'\].*/\$config_directories\['staging'\] = '$CONFIG_DIR\/staging';/g" $SETTINGS_FILES_PATH
fi

#== Update permission
sudo chown -R www:www-data $APP_ROOT
sudo chown -R www-data:www-data STATIC_FILES_PATH
