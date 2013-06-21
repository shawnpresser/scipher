require 'open3'
require 'tempfile'
include Open3

def usage()
  puts "                                                              "
  puts "  ruby scipher.rb enc [infile]                                "
  puts "                                                              "
  puts "    examples:                                                 "
  puts "                                                              "
  puts "    encrypt (hash) a password:                                "
  puts "    $ printf 'somepassword' > pwfile;                         "
  puts "    $ cat pwfile | ruby scipher.rb enc > hashfile             "
  puts "                                                              "
  puts "    decrypt (test) a password:                                "
  puts "    $ printf 'somepassword' > pwfile;                         "
  puts "    $ cat pwfile | ruby scipher.rb dec hashfile               "
  puts "                                                              "
  puts "    (if no output, then the password was correct)             "
  puts "                                                              "
  puts "    encrypt a file with a passphrase:                         "
  puts "    $ printf 'somepassword' > pwfile;                         "
  puts "    $ cat pwfile | ruby scipher.rb enc photo.jpg > photo.enc  "
  puts "                                                              "
  puts "    decrypt an encrypted file:                                "
  puts "    $ printf 'somepassword' > pwfile;                         "
  puts "    $ cat pwfile | ruby scipher.rb dec photo.enc > photo.jpg  "
  puts "                                                              "
  exit 1
end

if ARGV.length < 1 or (ARGV[0] == "dec" and ARGV.length < 2)
  usage
end

if STDIN.isatty
  puts "pipe a password into stdin"
  exit 2
end
pw = STDIN.read

infile = ""
if ARGV.length >= 2
  infile = ARGV[1]
end

if ARGV[0] == 'enc'
  o, e, s = Open3.capture3("scipher enc "+infile, :stdin_data=>pw)
elsif ARGV[0] == 'dec'
  o, e, s = Open3.capture3("scipher dec "+infile, :stdin_data=>pw)
else
  usage
end

if s.success?
  STDOUT.binmode; print o
else
  raise e
end

