import { useState } from "react";
import { motion, AnimatePresence } from "motion/react";
import { Search, CheckSquare, Square, Filter, Eye, EyeOff } from "lucide-react";
import { LogChannel, STANDARD_CHANNELS } from "./LoggingTypes";
import { Input } from "../ui/input";
import { Badge } from "../ui/badge";
import { ScrollArea } from "../ui/scroll-area";

interface ChannelSelectorProps {
  selectedChannels: string[];
  onChannelToggle: (channelId: string) => void;
  onSelectAll: () => void;
  onDeselectAll: () => void;
}

export function ChannelSelector({
  selectedChannels,
  onChannelToggle,
  onSelectAll,
  onDeselectAll
}: ChannelSelectorProps) {
  const [searchTerm, setSearchTerm] = useState("");
  const [categoryFilter, setCategoryFilter] = useState<string | null>(null);

  const categories = [
    { id: 'engine', name: 'Engine', color: '#00FF00' },
    { id: 'fuel', name: 'Fuel', color: '#FF00FF' },
    { id: 'ignition', name: 'Ignition', color: '#FF9900' },
    { id: 'sensors', name: 'Sensors', color: '#0099FF' },
    { id: 'calculated', name: 'Calculated', color: '#00AAFF' }
  ];

  const filteredChannels = STANDARD_CHANNELS.filter(channel => {
    const matchesSearch = channel.name.toLowerCase().includes(searchTerm.toLowerCase()) ||
                         channel.unit.toLowerCase().includes(searchTerm.toLowerCase());
    const matchesCategory = !categoryFilter || channel.category === categoryFilter;
    return matchesSearch && matchesCategory;
  });

  const selectedCount = selectedChannels.length;
  const totalCount = STANDARD_CHANNELS.length;

  return (
    <div className="h-full flex flex-col bg-[#1a1a1a] border-r border-[#333333]">
      {/* Header */}
      <div className="p-4 border-b border-[#333333]">
        <div className="flex items-center justify-between mb-3">
          <div>
            <h3 className="text-[#E6EEF3]">Channel Selector</h3>
            <p className="text-xs text-[#888888] mt-1">
              {selectedCount} of {totalCount} selected
            </p>
          </div>
          <div className="flex items-center gap-2">
            <button
              onClick={onSelectAll}
              className="text-xs px-2 py-1 bg-[#00C4B4] hover:bg-[#00D4C4] text-black rounded transition-colors"
            >
              All
            </button>
            <button
              onClick={onDeselectAll}
              className="text-xs px-2 py-1 bg-[#333333] hover:bg-[#444444] text-[#E6EEF3] rounded transition-colors"
            >
              None
            </button>
          </div>
        </div>

        {/* Search */}
        <div className="relative">
          <Search className="absolute left-2 top-1/2 -translate-y-1/2 w-4 h-4 text-[#666666]" />
          <Input
            value={searchTerm}
            onChange={(e) => setSearchTerm(e.target.value)}
            placeholder="Search channels..."
            className="pl-8 bg-[#0a0a0a] border-[#333333] text-[#E6EEF3] text-sm"
          />
        </div>

        {/* Category filters */}
        <div className="flex flex-wrap gap-2 mt-3">
          <button
            onClick={() => setCategoryFilter(null)}
            className={`text-xs px-2 py-1 rounded transition-colors ${
              categoryFilter === null
                ? 'bg-[#00C4B4] text-black'
                : 'bg-[#2a2a2a] text-[#888888] hover:bg-[#333333]'
            }`}
          >
            All
          </button>
          {categories.map(cat => (
            <button
              key={cat.id}
              onClick={() => setCategoryFilter(cat.id)}
              className={`text-xs px-2 py-1 rounded transition-colors ${
                categoryFilter === cat.id
                  ? 'text-black'
                  : 'bg-[#2a2a2a] text-[#888888] hover:bg-[#333333]'
              }`}
              style={{
                backgroundColor: categoryFilter === cat.id ? cat.color : undefined
              }}
            >
              {cat.name}
            </button>
          ))}
        </div>
      </div>

      {/* Channel list */}
      <ScrollArea className="flex-1">
        <div className="p-2 space-y-1">
          {filteredChannels.map((channel) => {
            const isSelected = selectedChannels.includes(channel.id);
            return (
              <motion.button
                key={channel.id}
                onClick={() => onChannelToggle(channel.id)}
                className={`w-full p-3 rounded border text-left transition-all ${
                  isSelected
                    ? 'bg-[#2a2a2a] border-[#00C4B4]'
                    : 'bg-[#1a1a1a] border-[#333333] hover:border-[#444444]'
                }`}
                whileHover={{ scale: 1.02 }}
                whileTap={{ scale: 0.98 }}
              >
                <div className="flex items-start justify-between gap-3">
                  <div className="flex-1 min-w-0">
                    <div className="flex items-center gap-2 mb-1">
                      {isSelected ? (
                        <CheckSquare className="w-4 h-4 text-[#00C4B4] flex-shrink-0" />
                      ) : (
                        <Square className="w-4 h-4 text-[#666666] flex-shrink-0" />
                      )}
                      <span className="text-sm text-[#E6EEF3] truncate">
                        {channel.name}
                      </span>
                    </div>
                    <div className="flex items-center gap-2 ml-6">
                      <Badge
                        variant="outline"
                        className="text-xs border-[#444444] text-[#888888]"
                      >
                        {channel.unit}
                      </Badge>
                      <span className="text-xs text-[#666666]">
                        {channel.min} - {channel.max}
                      </span>
                    </div>
                  </div>
                  
                  {/* Color indicator */}
                  <div
                    className="w-3 h-3 rounded-full flex-shrink-0 mt-1"
                    style={{ backgroundColor: channel.color }}
                  />
                </div>
              </motion.button>
            );
          })}

          {filteredChannels.length === 0 && (
            <div className="text-center py-8 text-[#666666]">
              <Filter className="w-8 h-8 mx-auto mb-2 opacity-50" />
              <p className="text-sm">No channels found</p>
              <p className="text-xs mt-1">Try adjusting your search or filters</p>
            </div>
          )}
        </div>
      </ScrollArea>

      {/* Footer stats */}
      <div className="p-3 border-t border-[#333333] bg-[#0a0a0a]">
        <div className="grid grid-cols-2 gap-3 text-xs">
          <div>
            <div className="text-[#666666]">Selected</div>
            <div className="text-[#00C4B4] font-mono">{selectedCount}</div>
          </div>
          <div>
            <div className="text-[#666666]">Est. Rate</div>
            <div className="text-[#E6EEF3] font-mono">
              {selectedCount > 0 ? `${Math.max(10, 50 - selectedCount * 2)}Hz` : '—'}
            </div>
          </div>
        </div>
        <p className="text-xs text-[#666666] mt-2">
          More channels = lower sample rate
        </p>
      </div>
    </div>
  );
}
