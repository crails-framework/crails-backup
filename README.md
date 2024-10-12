# crails-backup

A command line interface tool managing several aspects of backup management.

- [Backups](#performing-a-backup) and restores from filesystem and databases (PostgreSQL, MySQL, MongoDB)
- [Schedules](#scheduling-backups) backups using [cron](https://en.wikipedia.org/wiki/Cron)
- [Performs](#backends) either incremental backups using [bup](https://bup.github.io/) or snapshots using `tar`.
- [Manages](#backup-retention) the lifetime of backups files.

## Installing

crails-backup uses the [build2](https://www.build2.org/) build system, and you may use this git repository as a build2 package.

We also provide an install script which should take care of everything. Just run the following command:

```
bash <(curl -s "https://raw.githubusercontent.com/crails-framework/crails-backup/master/install.sh")
```

## How to use
### Performing a backup

With crails-backup, your backups consist of a name and a list of backable resources. The latter can either be filesystem pathes, or databases represented using database URLs. The following command would create a backup named `my_wordpress` backing a mysql database and a single folder.

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

By default, backup are stored in the `/opt/crails-backup` folder. Since we named our backup `my_wordpress`, the backup archive would be found in `/opt/crails-backup/my_wordpress` folder. Each backup executed under the same name will be assigned a number, and stored in the corresponding folder: as new backups as stored, older backups are removed, according to the [retention strategy](#backup-retention) you've chosen.

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

### Wiping backups

You can remove all the backups for a given project by using the wipe
command:

```
crails-backup wipe -n my_wordpress
```

## Storage

By default, backups are stored in the `/opt/crails-backup` folder. This
behavior can be changed by setting the `CRAILS_BACKUP_PATH` environment
variable.

The bup backend also supports storing backups on a remote server (see
the [bup](#bup) section).

#### Backup retention

When triggering a new backup, crails-backup also checks the available
backup and remove backups which are too old. By default, backups are
considered expired after 31 days.

You may change how long backups are kept around by setting the
`BACKUP_MAX_RETENTION` variable.

#### Backup long-term retention

You may also set an option for long-term backups. Long-term retention
happens after a duration defined by the `BACKUP_LONGTERM_STARTS_AFTER`
environment variable has passed. By default, that value is of 1 day.

Once the backup are older than `BACKUP_LONGTERM_STARTS_AFTER`,
crails-backup will only keep one backup for every period of
24h. That value can be customized using the
`BACKUP_LONGTERM_PERIODICITY` environment variable.

#### Backup retention time format

The value of the environment variables used by the backup retention
system can be expressed in seconds. You may also express it as such:

```
BACKUP_MAX_RETENTION="365d" # keep backups for one year
BACKUP_LONGTERM_STARTS_AFTER="31d" # starts long-term storage after one month
BACKUP_LONGTERM_PERIODICITY="4d2h30m' # long-term storage keeps one backup every 4 days, 2 hours and 30 minutes
```

#### Using environment variables in scheduled backups

When scheduling backups using `crails-backup add`, you need to set
the environment variables in your _crontab_. Use the `crontab -e`
command and append the environment variables at the top of the file.

For instance:
```
LD_LIBRRAY_PATH="/usr/local/lib"
CRAILS_BACKUP_PATH="/backup-store"
CRAILS_BACKUP_SERVERNAME="bup-server-hostname"
BACKUP_MAX_RETENTION="365d"
BACKUP_LONGTERM_STARTS_AFTER="31d"
BACKUP_LONGTERM_PERIODICITY="2d"
5 4 * * /usr/bin/crails-backup backup -n my_wordpress -d 'mysql://user:password@localhost/name' -f /srv/my-wordpress/wp-content #name=my_wordpress
```

## Backends

crails-backup provides two different backends:

- Incremental backups using [bup](https://bup.github.io/)
- Snapshots backups using tar

If the bup command is available, the bup backend will be used by default.
Otherwise, it will fallback to the tar backend.

### Bup
#### Remote storage

The bup backend can run the bup command with the `-r` option, storing the backups
at a remote server (see bup's [README](https://github.com/bup/bup#getting-started)
for details on how to sett up a bup server). The `SERVERNAME` can be customized
using the `CRAILS_BACKUP_SERVERNAME` environment variable. The remote server
will store the backups according to the `CRAILS_BACKUP_PATH` environment variable
from the client process performing the backup.

### Tar
#### Compressors

The tar backend compresses the snapshot using gzip. You may use other compressors
by setting the `CRAILS_BACKUP_COMPRESSOR` environment variable. Available values
are `gzip` `bzip2` and `xz`.
