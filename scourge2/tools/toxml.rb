#!/usr/local/bin/ruby
if ARGV.size < 2 
  puts "Usage: toxml <infile.cfg> <outfile.xml>"
  exit 1
end

infile = ARGV[0]
outfile = ARGV[1]

class CfgNode
  attr_accessor :name
  attr_accessor :id
  attr_accessor :props
  attr_accessor :translate
  attr_accessor :children
  attr_accessor :parent
  attr_accessor :super_node

  def initialize(name, id=nil)
    self.name = name
    self.id = id
    self.props = []
    self.translate = []
    self.children = []
    self.parent = nil
    self.super_node = nil
  end

  def fill_missing_props
    unless super_node == nil
      super_node.props.each { |pair|
        unless self.props.any? { |p| p[0] == pair[0] }
          self.props << pair
        end
      }
    end
  end

  def to_xml(fout, indent="")
    fout.puts "#{indent}<#{name}>\n"
    props.each { |pair|
      fout.puts "#{indent}  <#{pair[0]}#{translate.include?(pair[0]) ? " translate=\"true\"" : ""}>#{pair[1]}</#{pair[0]}>\n"
    }
    children.each { |child|
      child.to_xml(fout, indent + "  ")
    }
    fout.puts "#{indent}</#{name}>\n"
  end
end

@nodes = {}
@root = nil
@current_node = @root

def parse_tag(tag)
  if tag[0,1] == "/"
    @current_node.fill_missing_props
    @current_node = @current_node.parent
  else
    node = nil
    if tag =~ /(.*?),(.*)/
      node = CfgNode.new($1.strip)
      node.super_node = @nodes[$2.strip]
    else
      node = CfgNode.new(tag)
    end
    if @current_node != nil
      node.parent = @current_node
      @current_node.children << node
    else
      @root = node
    end
    @current_node = node
  end
end

def parse_prop(name, value)
  translate = false
  if value =~ /_\( *\"(.*?)\" *\)/
    translate = true
    value = $1
  elsif value =~ /\"(.*?)\"/
    value = $1
  end
  if name == "id"
    @current_node.id = value
    @nodes[@current_node.id] = @current_node
  else
    @current_node.props << [name, value]
    @current_node.translate << name if translate
  end
end

def parse(line, line_count)
  line.strip!
  if line =~ /#/ or line.size == 0
      return 
  end
  if line =~ /\[(.*?)\]/
    parse_tag($1.strip)
  elsif line =~ /(.*?)=(.*)/
    parse_prop($1.strip, $2.strip)
  else
    puts "Syntax error on line #{line_count}: #{line}"
  end
end

File.open(infile, "r") { |fin|
  line_count = 0
  while((line = fin.gets()) != nil)
    parse(line, line_count)
    line_count += 1
  end
}

File.open(outfile, "w") { |fout|
  @root.to_xml fout
}
