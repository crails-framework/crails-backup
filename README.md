# crails-backup

A command line interface tool managing several aspects of backup management.

## Installing

crails-backup uses the [build2](https://www.build2.org/) build system, and you may use this git repository as a build2 package.

We also provide an install script which should take care of everything. Just run the following command:

```
bash <(curl -s "https://raw.githubusercontent.com/crails-framework/crails-backup/master/install.sh")
```

## How to use
### Performing a backup

With crails-backup, your backups consist of a name and a list of backable resource. The latter can either be a path from the filesystem, or databases, represented using database URLs. The following command would create a backup named `my_wordpress` backing a mysql database and a single folder.

```
crails-backup backup \
  -n my_wordpress \
  -d 'mysql://user:password@localhost/name' \
  -f /srv/my-wordpress/wp-content 
```

A single backup can also back multiple databases or paths at once:

```
crails-backup backup \
  -n my_wordpress \
  -d 'mysql://user:password@localhost/name' \
     'postgres://user:password@localhost/name' \
  -f /srv/my-wordpress/wp-content \
     /srv/my-wordpress/wp-config.php
```

By default, backup are stored in the `/opt/crails-backup` folder. Since we named our backup `my_wordpress`, the backup archive would be found in `/opt/crails-backup/my_wordpress` folder. Each backup executed under the same name will be assigned a number, and stored in the corresponding folder: as new backups as stored, older backups are removed, according to the [storing strategy](#notyet) you've chosen.

The backup storage folder can be customized using the `CRAILS_BACKUP_PATH` environment variable.

### Scheduling backups

Instead of directly performing a backup, crails-backup can also be used to schedule periodic backups:

```
crails-backup add \
  -n my_wordpress \
  -d 'mysql://user:password@localhost/name' \
  -f /srv/my-wordpress/wp-content \
  -s '5 4 * *'
```

This will use the cron system service to periodically perform the backup. The periodicity can be customized using the `-s` option, by providing a [cron schedule expression](https://crontab.guru/).

If you wish to prevent further backups, you can unschedule them using the `remove` command:

```
crails-backup remove \
  -n my_wordpress
```

### Listing available backups

You may list the backups available for restore using the `list` command:

```
crails-backup list -n my_wordpress
```

This will display a list of backup ids, along as the date and time at which the backup was performed. You may then use one of the backup id to perform a restore action.

### Restoring backups

Once you've figured out which backup you want to restore, you may do so using the following command:

```
crails-backup restore -n my_wordpress --id 1
```
