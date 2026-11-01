#!/bin/bash
# ---------------------------------------------------------------------
# Copyright (C) 2025 DevPanel
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

  #== Import database
if [[ $(mysql -h$DB_HOST -P$DB_PORT -u$DB_USER -p$DB_PASSWORD $DB_NAME -e "show tables;") == '' ]]; then
  if [[ -f "$APP_ROOT/.devpanel/dumps/db.sql.tgz" ]]; then
    echo  'Extract mysql files ...'
    SQLFILE=$(tar tzf $APP_ROOT/.devpanel/dumps/db.sql.tgz)
    tar xzf "$APP_ROOT/.devpanel/dumps/db.sql.tgz" -C /tmp/
    mysql -h$DB_HOST -P$DB_PORT -u$DB_USER -p$DB_PASSWORD $DB_NAME --init-command="SET SESSION sql_require_primary_key=0;" < /tmp/$SQLFILE
    rm /tmp/$SQLFILE
  fi
fi

if [[ -n "${DB_SYNC_VOL:-}" ]]; then
  if [[ ! -f "/var/www/build/.devpanel/init-container.sh" ]]; then
    sudo chown -R 1000:1000 /var/www/build
    sudo rm -rf /var/www/build/lost+found
    echo 'Sync volume...'
    sudo rsync -a --delete --ignore-existing --exclude .devpanel/dumps $APP_ROOT/ /var/www/build
  fi
fi
