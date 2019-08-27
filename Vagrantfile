Vagrant.configure("2") do |config|
  config.vm.box = "bento/ubuntu-18.04"
  config.vm.synced_folder ".", "/home/vagrant/linux_tcpip"
  config.vm.network "private_network", ip: "192.168.100.2"

  config.vm.provision "shell", inline: <<-SHELL
    sudo apt update
    sudo apt -y upgrade
    sudo apt install -y git build-essentials traceroute cmake gdb
  SHELL
end

