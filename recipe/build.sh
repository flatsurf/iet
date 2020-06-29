#!/bin/bash
set -ex

source $RECIPE_DIR/environment.sh

if [[ "$component" != "intervalxt" ]]; then
  cd $component
fi

$SNIPPETS_DIR/autoconf/run.sh
$SNIPPETS_DIR/make/run.sh

$SNIPPETS_DIR/clang-format/run.sh
$SNIPPETS_DIR/todo/run.sh
$SNIPPETS_DIR/codecov/run.sh
