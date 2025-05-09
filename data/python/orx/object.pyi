from clock import Clock
from guid import Guid
from vector import Vector

class Object:
  def __new__(cls, name: str | Guid) -> Object: ...
  def __eq__(self, other: Object) -> bool: ...
  def __hash__(self) -> int: ...
  def __str__(self) -> str: ...

  def delete(self) -> None: ...
  def get_guid(self) -> Guid: ...

  def enable(self, state: bool, recursive: bool = False) -> None: ...
  def is_enabled(self) -> bool: ...

  def pause(self, state: bool, recursive: bool = False) -> None: ...
  def is_paused(self) -> bool: ...

  def set_owner(self, owner: Object | None) -> None: ...
  def get_owner(self) -> Object | None: ...

  def find_owned_child(self, path: str) -> Object | None: ...

  def set_clock(self, clock: Clock, recursive: bool = False): ...
  def get_clock(self) -> Clock | None: ...

  def set_flip(self, flip_x: bool, flip_y: bool) -> None: ...
  def get_flip(self) -> tuple[bool, bool]: ...

  def set_position(self, position: Vector, world: bool = False) -> None: ...
  def get_position(self, world: bool = False) -> Vector: ...

  def set_rotation(self, rotation: float, world: bool = False) -> None: ...
  def get_rotation(self, world: bool = False) -> float: ...

  def set_scale(self, scale: Vector, world: bool = False) -> None: ...
  def get_scale(self, world: bool = False) -> Vector: ...

  def set_parent(self, parent: Object | None) -> None: ...
  def get_parent(self) -> Object | None: ...

  def find_child(self, path: str) -> Object | None: ...

  def attach(self, parent: Object) -> None: ...
  def detach(self) -> None: ...

  def log_parents(self) -> None: ...

  def set_anim_frequency(self, frequency: float, recursive: bool = False) -> None: ...
  def get_anim_frequency(self) -> float: ...
  def set_anim_time(self, time: float, recursive: bool = False) -> None: ...
  def get_anim_time(self) -> float: ...
  def set_current_anim(self, name: str, recursive: bool = False) -> None: ...
  def get_current_anim(self) -> str: ...
  def set_target_anim(self, name: str, recursive: bool = False) -> None: ...
  def get_target_anim(self) -> str: ...

  def set_speed(self, speed: Vector, relative = False) -> None: ...
  def get_speed(self, relative: bool = False) -> Vector: ...

  def set_angular_velocity(self, velocity: float) -> None: ...
  def get_angular_velocity(self) -> float: ...

  def set_custom_gravity(self, dir: Vector) -> None: ...
  def get_custom_gravity(self) -> Vector: ...

  def get_mass(self) -> float: ...
  def get_mass_center(self) -> Vector: ...

  def apply_torque(self, torque: float) -> None: ...
  def apply_force(self, force: Vector, point: Vector) -> None: ...
  def apply_impulse(self, impulse: Vector, point: Vector) -> None: ...

  def set_text_string(self, s: str) -> None: ...
  def get_text_string(self) -> str: ...

  def add_fx(self, name: str, recursive: bool = False, unique: bool = True, propagation_delay: float = 0) -> None: ...
  def remove_fx(self, name: str, recursive: bool = False) -> None: ...
  def remove_all_fxs(self, recursive: bool = False) -> None: ...

  def add_sound(self, name: str) -> None: ...
  def remove_sound(self, name: str) -> None: ...
  def remove_all_sounds(self) -> None: ...
  def set_volume(self, volume: float) -> None: ...
  def set_pitch(self, pitch: float) -> None: ...
  def set_panning(self, panning: float, mix: bool) -> None: ...
  def play(self) -> None: ...
  def stop(self) -> None: ...
  def add_filter(self, name: str) -> None: ...
  def remove_last_filter(self) -> None: ...
  def remove_all_filters(self) -> None: ...

  def set_shader_from_config(self, name: str | None, recursive: bool = False) -> None: ...
  def enable_shader(self, enabled: bool = True) -> None: ...
  def is_shader_enabled(self) -> bool: ...

  def add_time_line_track(self, name: str, recusive: bool = False) -> None: ...
  def remove_time_line_track(self, name: str, recursive: bool = False) -> None: ...
  def enable_time_line(self, enabled: bool = True) -> None: ...
  def is_time_line_enabled(self) -> bool: ...

  def add_trigger(self, name: str, recursive: bool = False) -> None: ...
  def remove_trigger(self, name: str, recursive: bool = False) -> None: ...
  def fire_trigger(self, name: str, refinement: list[str] | None = None, recursive: bool = False) -> None: ...

  def get_name(self) -> str: ...

  def set_rgb(self, rgb: Vector, recursive: bool = False) -> None: ...
  def get_rgb(self) -> Vector: ...

  def set_alpha(self, alpha: float, recursive: bool = False) -> None: ...
  def get_alpha(self) -> float: ...

  def set_life_time(self, life_time: float | str | None) -> None: ...
  def get_life_time(self) -> float | None: ...

  def get_active_time(self) -> float: ...
  def reset_active_time(self, recursive: bool = False) -> None: ...

def create_from_config(name: str) -> Object | None: ...
def from_guid(guid: Guid) -> Object | None: ...

def raycast(begin: Vector, end: Vector, self_flags: int, check_mask: int, early_exit: bool = False) -> tuple[Object, Vector, Vector] | None: ...
