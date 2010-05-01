#!/usr/local/bin/ruby
# Script to create a texture atlas. Requires ruby and 'convert' from ImageMagick.
# To run it do:
# ruby ./tools/canvas.rb image_list.txt 512 512 64 64 data/textures/atlas/outdoor.png
# where:
# image_list.txt is a text file with 1 image path per line
# 512 512 is the width/height of the atlas image
# 64 64 is the width/height of a tile in the atlas
# the final arg is the path to where the resulting image will go
#
# The script currently only supports atlases where each image is the same size.
#

if ARGV.size != 6
  puts <<EOS
Usage: ruby ./canvas.rb <image list file> <canvas width> <canvas height> <tile width> <tile height> <resulting image file name>
EOS
  exit 1
end

imgfile = ARGV[0]
img_w = ARGV[1].to_i
img_h = ARGV[2].to_i
tile_w = ARGV[3].to_i
tile_h = ARGV[4].to_i
canvas_name = ARGV[5]

cmd = "convert -size #{img_w}x#{img_h} xc:none"
x = 0
y = 0
line = ""
File.open(imgfile, "r") do |file|
  while (line = file.gets) != nil
    cmd += " -draw \"image over #{x},#{y} #{tile_w},#{tile_h} '#{line.strip}'\""
    x += tile_w
    if x >= img_w
      x = 0
      y += tile_h
      break if y >= img_h
    end
  end
  cmd += " #{canvas_name}"
  puts "Creating #{canvas_name}..."
  `#{cmd}`
end
