module BNRD
  class Debugger
    def initialize
      puts "BNRD.Debugger"
      puts OPTIONS
      path = OPTIONS.script_path
      iseq = RubyVM::InstructionSequence.compile_file(path)
      #puts iseq.disasm

      BNRD.add_breakpoint(OPTIONS.bp_lineno, iseq)

      TracePoint.new(:specified_line){|tp|
        puts "#{tp.path}:#{tp.lineno}"
        variables = tp.binding.local_variables

        puts "Breakpoint:"
        variables.each do |variable|
          puts "  #{variable}: #{tp.binding.local_variable_get variable}"
        end
        line = gets
        while (line.chomp != 'cont')
          puts "Enter `cont` (#{line.chomp} is not cont)"
          line = gets
        end

      }.enable{
        iseq.eval
      }
    end
  end
end
