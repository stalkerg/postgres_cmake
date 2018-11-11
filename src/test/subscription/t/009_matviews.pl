# Test materialized views behavior
use strict;
use warnings;
use PostgresNode;
use TestLib;
use Test::More tests => 1;

my $node_publisher = get_new_node('publisher');
$node_publisher->init(allows_streaming => 'logical');
$node_publisher->start;

my $node_subscriber = get_new_node('subscriber');
$node_subscriber->init(allows_streaming => 'logical');
$node_subscriber->start;

my $publisher_connstr = $node_publisher->connstr . ' dbname=postgres';
my $appname           = 'replication_test';

$node_publisher->safe_psql('postgres',
	"CREATE PUBLICATION mypub FOR ALL TABLES;");
$node_subscriber->safe_psql('postgres',
	"CREATE SUBSCRIPTION mysub CONNECTION '$publisher_connstr application_name=$appname' PUBLICATION mypub;"
);

$node_publisher->safe_psql('postgres',
	q{CREATE TABLE test1 (a int PRIMARY KEY, b text)});
$node_publisher->safe_psql('postgres',
	q{INSERT INTO test1 (a, b) VALUES (1, 'one'), (2, 'two');});

$node_subscriber->safe_psql('postgres',
	q{CREATE TABLE test1 (a int PRIMARY KEY, b text);});

$node_publisher->wait_for_catchup($appname);

# Materialized views are not supported by logical replication, but
# logical decoding does produce change information for them, so we
# need to make sure they are properly ignored. (bug #15044)

# create a MV with some data
$node_publisher->safe_psql('postgres',
	q{CREATE MATERIALIZED VIEW testmv1 AS SELECT * FROM test1;});
$node_publisher->wait_for_catchup($appname);

# There is no equivalent relation on the subscriber, but MV data is
# not replicated, so this does not hang.

pass "materialized view data not replicated";

$node_subscriber->stop;
$node_publisher->stop;
