#!/bin/bash -

ctags -R -n -f cppcomplete.tags --fields=+ai --C++-types=+p *
