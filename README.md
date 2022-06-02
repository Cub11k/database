# Database

Simple client-server non-relational database

Internal structure consists of two `red black trees` and one `std::unordered_map`

## Install

```bash
git clone https://github.com/Cub1tor/database.git
cd database
```
---

## Build

```bash
make
# or
make all

# to remove build files use
make clean
```
---

## Usage

```bash
./server
./client {Address} {Port}
# for example
./client localhost 8080
```
---

## Database interface

#### Server

* `exit` - stop server

#### Client


* `INSERT name=STRING group=INT rating=DOUBLE info=STRING END`
* `UPDATE {name=STRING group=INT rating=DOUBLE info=STRING} WHERE {Criteria} END`
* `DELETE {Criteria} END`
* `SELECT {Criteria} END`
* `RESELECT {Criteria} END`
* `PRINT <count=INT> {name group rating *} SORT {ASC|DESC} {name group rating} END`
* `LOAD file=FILENAME END`
* `DUMP file=FILENAME END`

**Criteria**
* list of _**Entries**_ connected by `AND|OR`

**Entry**
* `key=list of Values divided by ','`

**Keys**
* `name`
* `group`
* `rating`

**Values**
* `value` &#8594; equal
* `regex with asterisks` &#8594; `*` is any sequence of symbols, only for name
* `*-value` &#8594; less or equal, only for group and rating
* `value-*` &#8594; more or equal, only for group and rating
* `value1-value2` &#8594; between, only for group and rating

#### Examples

```bash
INSERT name="Cub1tor" group=204 rating=4.37 info="C++ programmer" END
UPDATE name=Cub1tor rating=4.37 info="C++ programmer" WHERE name=Cub* AND group=204 END
DELETE group=210 END
SELECT * END
RESELECT rating=4.2-5 END
PRINT count=10 name group SORT DESC group END
PRINT count=5 * SORT ASC group name rating END
LOAD file=load.json END
DUMP file=dump.json END
```
---