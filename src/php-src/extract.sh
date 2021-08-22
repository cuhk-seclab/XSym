#!/bin/sh
find -name *\.o | xargs -I 'aa' extract-bc 'aa'
