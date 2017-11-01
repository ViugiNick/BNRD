require 'ostruct'

module BNRD
  OPTIONS = OpenStruct.new(
      :script_path => ENV['BNRD_SCRIPT_PATH'],
  )

  def OPTIONS.set_env
    ENV['BNRD_SCRIPT_PATH'] = self.script_path.to_s
  end
end
