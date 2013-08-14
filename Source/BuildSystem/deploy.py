#! /usr/bin/python
#
# Confidential Information of Telekinesys Research Limited (t/a Havok). Not for
# disclosure or distribution without Havok's prior written consent. This
# software contains code, techniques and know-how which is confidential and
# proprietary to Havok. Product and Trade Secret source code contains trade
# secrets of Havok. Havok Software (C) Copyright 1999-2013 Telekinesys Research
# Limited t/a Havok. All Rights Reserved. Use of this software is subject to
# the terms of an end user license agreement.
#

"""
deploy.py - Deploy the plugins specified on the command line to $(VISION_SDK)
"""

import sys
import os
import shutil

from optparse import OptionParser


COMMAND_LINE_OPTIONS = (
    (('-s', '--source',),
     {'action': 'store',
      'dest': 'source',
      'help': "Source binary folder"}),
    (('-q', '--quiet',),
     {'action': 'store_false',
      'dest': 'verbose',
      'default': True,
      'help': "Don't print out status updates"}))

CUSTOM_PLUGINS = ['SpriteShapeEditor', 'SpriteShapeManaged', 'SpriteShapeEnginePlugin']


def main():
    """
    Deploy the plugins to the VISION_SDK folder
    """

    parser = OptionParser('')
    for options in COMMAND_LINE_OPTIONS:
        parser.add_option(*options[0], **options[1])
    (options, _) = parser.parse_args()
    
    source_directory = options.source
    if not source_directory and len(sys.argv) > 1:
        source_directory = sys.argv[len(sys.argv) - 1]

    success = False

    if not 'VISION_SDK' in os.environ:
        print("Missing VISION_SDK environment variable!")

    destination_directory = os.environ['VISION_SDK']

    if not source_directory:
        print("Didn't specify binary source directory!")
    else:
        source_directory = os.path.abspath(source_directory)
        bin_index = source_directory.rfind('Bin')
        bin_path = source_directory[bin_index:]
        output_path = os.path.join(destination_directory, bin_path)

        for filename in os.listdir(source_directory):
            for plugin in CUSTOM_PLUGINS:
                if plugin in filename:
                    source_file = os.path.join(source_directory, filename)
                    destination_file = os.path.join(output_path, filename)

                    if os.path.exists(destination_file) and\
                       (os.stat(source_file).st_mtime - os.stat(destination_file).st_mtime) > 1:
                        try:
                            shutil.copy2(source_file, output_path)
                            print('Updating %s...' % destination_file)
                        except IOError:
                            print('Failed to update %s...' % destination_file)
        success = True

    return success

if __name__ == "__main__":
    SUCCESS = main()
    sys.exit(0 if SUCCESS else 1)