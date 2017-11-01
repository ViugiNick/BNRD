module BNRD
  class TypeTracker
    include Singleton

    def initialize(path)
      iseq = RubyVM::InstructionSequence.compile_file(path)

      BNRD.add_breakpoint(path, 1, iseq)

      TracePoint.new(:specified_line){|tp|
        puts "Hi, bitch"
      }.enable{
        iseq.eval
      }
    end
  end
end
