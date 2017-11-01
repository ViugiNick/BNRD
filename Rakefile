require "bundler/gem_tasks"
require "rake/extensiontask"
require 'rake/testtask'

BASE_TEST_FILE_LIST = Dir['test/**/test_*.rb']

task :build => :compile

Rake::ExtensionTask.new("bnrd") do |ext|
  ext.lib_dir = "lib/bnrd"
end

desc "Test bnrd."
Rake::TestTask.new(:test => [:clean, :compile]) do |t|
  t.libs += %w(./ext ./lib)
  t.test_files = FileList[BASE_TEST_FILE_LIST]
  t.verbose = true
end

task :test => :lib

task :default => [:clobber, :compile, :test]
